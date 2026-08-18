#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "LCD_I2C.h"

#define SENSOR_N        6
#define ADC_BASE_CH     2

#define PSD_CH          1
#define PSD_LOG_N       32
#define PSD_LOG_EVERY   40
#define PSD_LOG_BURST   8
#define PSD_LOG_AFTER   5
#define PSD_PAGES       (PSD_LOG_N / 8)

#define PAGE_READY      0
#define PAGE_JCT        1
#define PAGE_PSD_SUM    2
#define PAGE_PSD_0      3

#define SENSOR_REVERSE  0

#define LEFT_IS_A       0
#define LEFT_INVERT     1
#define RIGHT_INVERT    1

#define PWM_TOP         3199

#define DUTY_MIN_A      40
#define DUTY_MAX_A      100
#define DUTY_MIN_B      30
#define DUTY_MAX_B      82

#define IN1             PB0
#define IN2             PB1
#define IN3             PB2
#define IN4             PB3
#define ENA             PB5
#define ENB             PB6

#define IN_MASK         ((1 << IN1) | (1 << IN2) | (1 << IN3) | (1 << IN4))
#define EN_MASK         ((1 << ENA) | (1 << ENB))

#define KP              22
#define KI              1
#define KD              70
#define PID_SHIFT       100
#define I_LIMIT         2000
#define STEER_MAX       100
#define ERR_FULL        500
#define D_FILTER        4

#define SPEED_STEPS     4
static const uint8_t SPEED_TABLE[SPEED_STEPS] = { 15, 25, 35, 45 };

#define NORM_MAX        1000
#define LINE_FLOOR      300
#define LINE_MIN_SUM    200
#define CROSS_W         500
#define CROSS_N         5

#define WIDE_N_FIRST    3

#define WIDE_N_T        3
#define WIDE_T_IDX      2
#define WIDE_N          4

#define CROSS_LOCK      100
#define CROSS_CLEAR     20

#define JUNCTION_PERSIST 6
#define JUNCTION_DECAY   2

#define START_IGNORE    1800
#define JUNCTION_STEP   0

#define JUNCTION_MAX    5

static const int16_t JUNCTION_ERR[JUNCTION_MAX]   = {  500,  500,  500,    0, -500 };
static const uint8_t JUNCTION_DELAY[JUNCTION_MAX] = {   60,   85,   70,    0,    0 };
static const uint8_t JUNCTION_HOLD[JUNCTION_MAX]  = {   49,   85,   65,   10,   40 };

static const uint8_t JUNCTION_LANE[JUNCTION_MAX]  = {    0,    0,    0,    1,    0 };

static const uint8_t JUNCTION_PARK[JUNCTION_MAX]  = {    0,    0,    0,    0,    1 };

static const uint8_t JUNCTION_ALIGN[JUNCTION_MAX] = {    0,   40,    0,    0,    0 };

#define LANE_SPEED      25

#define LANE_PROBE      20

#define LANE_GRACE      80
#define LANE_BACK_TICKS 99

#define LANE_TURN_TICKS  36

#define LANE_STUCK_RUN  250
#define LANE_STUCK_N    2

#define LANE_IGNORE     600
#define LANE_OFF        0
#define LANE_FWD        1
#define LANE_BACK       2
#define LANE_TURN       3
#define LANE_ARC        4

#define PARK_IN         5
#define PARK_PAUSE      6
#define PARK_ADV        7
#define PARK_SPIN       8
#define PARK_WAIT       9
#define PARK_OUT        10
#define PARK_EXIT       11

#define PARK_KP_DIV     3
#define PARK_IN_GRACE   100
#define PARK_PAUSE_TICKS 100
#define PARK_ADV_GRACE  60
#define PARK_ADV_MAX    180
#define PARK_SPIN_TICKS 161
#define PARK_WAIT_TICKS 400
#define PARK_OUT_GRACE  80
#define PARK_EXIT_TICKS 40

#define LANE_ARC_STOP   100
#define LANE_ARC_STEER  22
#define LANE_ARC_MAX    900

#define LANE_EXIT_NARROW  2

#define LANE_EXIT_CENTER 220
#define LANE_EXIT_HOLD    2

#define LED_TH          150

#define CORNER_ERR      500

#define BRIGHT_TH       500
#define POLARITY_HOLD   40

#define STOP6_TICKS     940
#define STOP6_SEE       250
#define STOP6_MID_A     2
#define STOP6_MID_B     3

#define STOP6_OPEN       100
#define STOP6_OPEN_TICKS 400

#define KICK_TICKS      20
#define KICK_SPEED      100

#define MKICK_AFTER     3
#define MKICK_TICKS     8
#define MKICK_DUTY      85

#define LOST_STRAIGHT   120
#define LOST_SPIN       240
#define LOST_SPEED      30
#define LOST_TURN_ERR   250

#define EE_MAGIC        0xA5C3

#define ST_IDLE         0
#define ST_RUN          1
#define ST_CALIB        2

static volatile uint8_t tick = 0;

static uint16_t cal_min[SENSOR_N];
static uint16_t cal_max[SENSOR_N];
static int16_t  norm[SENSOR_N];
static int16_t  weight[SENSOR_N];

static uint8_t  bg_white = 1;
static uint8_t  pol_count = 0;

static int16_t  err_prev = 0;
static int16_t  shaped_prev = 0;
static int16_t  d_filt = 0;
static int32_t  err_integ = 0;
static int8_t   lost_dir = 1;
static uint16_t lost_cnt = 0;

static uint8_t  cross_hold = 0;
static int16_t  cross_err = 0;
static uint16_t cross_lock = 0;
static uint8_t  cross_armed = 1;
static uint8_t  cross_clear = 0;
static uint8_t  cross_count = 0;
static uint8_t  stop6_armed = 0;
static uint16_t stop6 = 0;
static uint8_t  psd_seen = 0;
static uint8_t  stop6_held = 0;
static uint8_t  stop6_done = 0;
static uint16_t psd_show = 0;
static uint16_t stop6_open = 0;
static uint16_t lcd_shown_psd = 0xFFFF;
static uint8_t  in_junction = 0;
static uint8_t  cross_delay = 0;
static uint8_t  cross_align = 0;
static uint8_t  cross_align_ok = 0;
static uint8_t  cross_pending = 0;
static uint8_t  lane_pending = 0;
static uint8_t  park_pending = 0;
static uint16_t park_sub = 0;
static uint8_t  lane_state = LANE_OFF;
static uint16_t lane_sub = 0;
static uint8_t  lane_turn_amt = 0;
static uint16_t lane_fwd_run = 0;
static uint8_t  lane_stuck = 0;
static uint8_t  lane_after_corner = 0;
static uint8_t  lane_arc_cleared = 0;
static uint16_t lane_arc_run = 0;
static uint8_t  lane_contacts = 0;
static uint8_t  lane_probe = 0;
static int16_t  lane_err0 = 0;
static int16_t  lane_drift = 0;
static uint16_t start_ignore = 0;
static uint8_t  lane_grace = 0;
static uint16_t lane_ignore = 0;
static int8_t   lane_dir = 0;
static uint8_t  psd_log[PSD_LOG_N];
static uint8_t  psd_head = 0;
static uint8_t  psd_n = 0;
static uint8_t  psd_div = 0;
static uint8_t  psd_lo = 255;
static uint8_t  psd_hi = 0;
static uint8_t  psd_wmax = 0;
static uint8_t  psd_armed = 0;

static uint8_t  show_page = 0;
static uint8_t  lcd_dirty = 1;
static uint8_t  lcd_shown_count = 255;
static uint8_t  junction_now = 0;
static uint8_t  junction_run = 0;
static int8_t   dir_a = 0;
static int8_t   dir_b = 0;
static uint8_t  mkick_a = 0;
static uint8_t  mkick_b = 0;
static uint8_t  junction_hit = 0;

static const int8_t POS[SENSOR_N] = { -5, -3, -1, 1, 3, 5 };

static uint16_t EEMEM ee_magic;
static uint16_t EEMEM ee_min[SENSOR_N];
static uint16_t EEMEM ee_max[SENSOR_N];

void INIT_tick(void);
void INIT_adc(void);
void INIT_motor(void);
void INIT_io(void);

uint16_t READ_adc(uint8_t ch);
void READ_sensors(void);
uint8_t UPDATE_polarity(void);
int16_t CONVERT_weightToError(uint8_t* on_line);

void SET_motorA(int16_t spd);
void SET_motorB(int16_t spd);
void SET_drive(int16_t left, int16_t right);
void SET_stop(void);
uint8_t CONVERT_spdToDuty(int16_t spd, uint8_t dmin, uint8_t dmax);

uint8_t LOAD_calibration(void);
void SAVE_calibration(void);
uint8_t READ_edge(uint8_t now, uint8_t* prev);
void UPDATE_leds(uint8_t state, uint8_t blink);
void UPDATE_lcd(uint8_t state, uint8_t spd);
static void PUT_line(uint8_t row, const char* src, uint8_t num, uint8_t show_num);
static void LCD_putCount(uint8_t n);
static void LCD_putPsd(uint16_t v);
static void LANE_handOff(void);
static uint8_t IS_centerAligned(void);
static void PUT_psdPage(uint8_t page);
static void PUT_psdSummary(void);
static void FMT_u8(char* dst, uint8_t v);
static void RESET_psdLog(void);
static void SAMPLE_psd(void);

#define SW1_ON()        (!(PINC & (1 << PINC0)))
#define SW2_ON()        (!(PINC & (1 << PINC1)))
#define SW3_ON()        (!(PIND & (1 << PIND2)))
#define SW4_ON()        (!(PIND & (1 << PIND3)))

int main(void)
{
    uint8_t  state = ST_IDLE;
    uint8_t  spd_step = 1;
    uint8_t  prev1 = 0, prev2 = 0, prev3 = 0, prev4 = 0;
    uint8_t  on_line = 0;
    uint8_t  blink = 0;
    uint16_t blink_cnt = 0;
    uint16_t kick_cnt = 0;
    int16_t  err = 0;

    MCUCSR |= (1 << JTD);
    MCUCSR |= (1 << JTD);

    INIT_motor();
    INIT_io();
    INIT_adc();

    if (LOAD_calibration())
    {
        PORTA = (uint8_t)~0x3F;
        _delay_ms(500);
    }
    else
    {
        for (uint8_t i = 0; i < 3; i++)
        {
            PORTA = 0x00;
            _delay_ms(120);
            PORTA = 0xFF;
            _delay_ms(120);
        }
    }

    PORTA = 0xFF;

    lcdInit();
    lcdClear();

    INIT_tick();

    sei();

    while (1)
    {
        if (!tick)
            continue;

        tick = 0;

        READ_sensors();
        UPDATE_polarity();
        err = CONVERT_weightToError(&on_line);

        if (READ_edge(SW1_ON(), &prev1))
        {
            lcd_dirty = 1;

            if (state == ST_RUN && stop6_held)
            {

                stop6_held = 0;
                stop6_done = 1;
                kick_cnt = KICK_TICKS;

                err_integ = 0;
                err_prev = 0;
                shaped_prev = 0;
                d_filt = 0;
                in_junction = 1;
                lost_cnt = 0;

                PUT_line(0, "JCT: ", 0, 0);
                PUT_line(1, "RUNNING", 0, 0);
                lcd_shown_count = 255;
                lcd_dirty = 0;
            }
            else if (state == ST_RUN)
            {
                state = ST_IDLE;
                SET_stop();
            }
            else if (state == ST_IDLE)
            {
                state = ST_RUN;
                err_prev = 0;
                shaped_prev = 0;
                d_filt = 0;
                err_integ = 0;
                lost_cnt = 0;
                cross_hold = 0;
                cross_delay = 0;
                lane_pending = 0;
                park_pending = 0;
                park_sub = 0;
                lane_state = LANE_OFF;
                lane_ignore = 0;
                lane_grace = 0;
                lane_after_corner = 0;
                lane_arc_cleared = 0;
                lane_arc_run = 0;
                junction_hit = 0;
                junction_run = 0;
                cross_armed = 1;
                cross_clear = 0;
                dir_a = 0;
                dir_b = 0;
                mkick_a = 0;
                mkick_b = 0;
                cross_lock = CROSS_LOCK;
                kick_cnt = KICK_TICKS;
                stop6_armed = 0;
                stop6 = 0;
                stop6_held = 0;
                stop6_done = 0;
                stop6_open = 0;
                psd_seen = 0;

                RESET_psdLog();

                PUT_line(0, "JCT: ", 0, 0);
                PUT_line(1, "RUNNING", 0, 0);
                lcd_shown_count = 255;
                start_ignore = START_IGNORE;
                lcd_dirty = 0;

                if (!JUNCTION_STEP)
                    cross_count = 0;
            }
        }

        if (READ_edge(SW2_ON(), &prev2))
        {
            lcd_dirty = 1;

            if (state == ST_CALIB)
            {
                SAVE_calibration();
                state = ST_IDLE;
            }
            else
            {
                SET_stop();
                state = ST_CALIB;
                psd_armed = 0;

                for (uint8_t i = 0; i < SENSOR_N; i++)
                {
                    cal_min[i] = 1023;
                    cal_max[i] = 0;
                }
            }
        }

        if (READ_edge(SW3_ON(), &prev3))
        {
            spd_step = (uint8_t)((spd_step + 1) % SPEED_STEPS);
            lcd_dirty = 1;
        }

        if (READ_edge(SW4_ON(), &prev4))
        {
            show_page = (uint8_t)((show_page + 1) % (PAGE_PSD_0 + PSD_PAGES));
            lcd_dirty = 1;
        }

        if (state == ST_RUN && stop6_armed && !stop6_held && !stop6_done)
        {
            if (stop6 > 0)
            {
                stop6--;
            }
            else if (IS_centerAligned())
            {
                stop6_held = 1;
                SET_stop();

                psd_armed = 1;

                psd_div = 0;
                psd_seen = 0;
                stop6_open = 0;

                PUT_line(0, "STOP   SW1=go", 0, 0);
                PUT_line(1, "PSD:", 0, 0);
                lcd_shown_psd = 0xFFFF;
            }
        }

        if (state == ST_CALIB)
        {
            for (uint8_t i = 0; i < SENSOR_N; i++)
            {
                uint16_t v = READ_adc((uint8_t)(ADC_BASE_CH + i));

                if (v < cal_min[i])
                    cal_min[i] = v;

                if (v > cal_max[i])
                    cal_max[i] = v;
            }

            if (++blink_cnt >= 100)
            {
                blink_cnt = 0;
                blink = !blink;
            }
        }
        else if (state == ST_RUN && stop6_held)
        {

            SET_stop();

            err_integ = 0;
            err_prev = 0;
            shaped_prev = 0;
            d_filt = 0;
            in_junction = 1;

            SAMPLE_psd();

            if (psd_show != lcd_shown_psd)
            {
                lcd_shown_psd = psd_show;
                LCD_putPsd(psd_show);
            }

            if (psd_seen && psd_show <= STOP6_OPEN)
            {
                if (stop6_open < STOP6_OPEN_TICKS)
                    stop6_open++;
            }
            else
            {
                stop6_open = 0;
            }

            if (stop6_open >= STOP6_OPEN_TICKS)
            {

                stop6_held = 0;
                stop6_done = 1;
                kick_cnt = KICK_TICKS;

                err_integ = 0;
                err_prev = 0;
                shaped_prev = 0;
                d_filt = 0;
                in_junction = 1;
                lost_cnt = 0;

                PUT_line(0, "JCT: ", 0, 0);
                PUT_line(1, "RUNNING", 0, 0);
                lcd_shown_count = 255;
                lcd_dirty = 0;
            }
        }
        else if (state == ST_RUN)
        {
            int16_t base = (int16_t)SPEED_TABLE[spd_step];
            int16_t steer;

            if (lane_state == LANE_FWD)
            {
                if (lane_grace > 0)
                {

                    lane_grace--;

                    SET_drive((int16_t)LANE_SPEED, (int16_t)LANE_SPEED);
                }
                else if (on_line || lane_probe > 0)
                {

                    uint8_t decided = 0;

                    if (lane_probe == 0)
                    {
                        lane_probe = LANE_PROBE;
                        lane_err0 = err;
                        lane_drift = 0;
                    }
                    else
                    {
                        lane_probe--;

                        if (on_line)
                        {
                            int16_t d = (int16_t)(err - lane_err0);

                            if (d < 0)
                                d = (int16_t)-d;

                            if (d > lane_drift)
                                lane_drift = d;
                        }

                        if (lane_probe == 0 || !on_line)
                            decided = 1;
                    }

                    if (!decided)
                    {
                        SET_drive((int16_t)LANE_SPEED, (int16_t)LANE_SPEED);
                    }
                    else
                    {
                        lane_probe = 0;

                        if (lane_contacts < 255)
                            lane_contacts++;

                        if (lane_ignore > 0)
                        {

                            lane_stuck = 0;
                        }
                        else if (lane_fwd_run < LANE_STUCK_RUN)
                        {
                            if (lane_stuck < 255)
                                lane_stuck++;
                        }
                        else
                        {
                            lane_stuck = 0;
                        }

                        lane_fwd_run = 0;

                        if (lane_stuck >= LANE_STUCK_N)
                            lane_after_corner = 1;

                        if (lane_after_corner && on_line && err < 0)
                        {

                            lane_state = LANE_ARC;
                            lane_sub = LANE_ARC_STOP;
                            lane_arc_cleared = 0;
                            lane_arc_run = 0;

                            if (!stop6_armed && !stop6_done)
                            {
                                stop6_armed = 1;
                                stop6 = STOP6_TICKS;
                            }

                            SET_stop();
                        }
                        else if (on_line)
                        {

                            lane_dir = (err > 0) ? (int8_t)-1 : (int8_t)1;
                            lane_turn_amt = LANE_TURN_TICKS;
                            lane_sub = LANE_BACK_TICKS;
                            lane_state = LANE_BACK;

                            SET_drive((int16_t)-LANE_SPEED, (int16_t)-LANE_SPEED);
                        }
                        else
                        {

                            SET_drive((int16_t)LANE_SPEED, (int16_t)LANE_SPEED);
                        }
                    }
                }
                else
                {
                    if (lane_fwd_run < 65535)
                        lane_fwd_run++;

                    SET_drive((int16_t)LANE_SPEED, (int16_t)LANE_SPEED);
                }
            }
            else if (lane_state == LANE_BACK)
            {
                lane_sub--;

                SET_drive((int16_t)-LANE_SPEED, (int16_t)-LANE_SPEED);

                if (lane_sub == 0)
                {
                    lane_state = LANE_TURN;
                    lane_sub = lane_turn_amt;
                }
            }
            else if (lane_state == LANE_TURN)
            {

                int16_t s = (lane_dir > 0) ? (int16_t)STEER_MAX : (int16_t)-STEER_MAX;

                lane_sub--;

                SET_drive((int16_t)(LANE_SPEED + s), (int16_t)(LANE_SPEED - s));

                if (lane_sub == 0)
                    lane_state = LANE_FWD;
            }
            else if (lane_state == LANE_ARC)
            {

                if (lane_sub > 0)
                {
                    lane_sub--;
                    SET_stop();
                }
                else
                {
                    if (lane_arc_run < 65535)
                        lane_arc_run++;

                    SET_drive((int16_t)(LANE_SPEED - LANE_ARC_STEER),
                              (int16_t)(LANE_SPEED + LANE_ARC_STEER));

                    if (!lane_arc_cleared)
                    {
                        if (!on_line)
                            lane_arc_cleared = 1;
                    }
                    else if (on_line)
                    {
                        LANE_handOff();
                    }

                    if (lane_arc_run >= LANE_ARC_MAX)
                    {
                        SET_stop();
                        state = ST_IDLE;
                    }
                }
            }
            else if (lane_state == PARK_IN)
            {

                if (park_sub > 0)
                {
                    int16_t s = (int16_t)(err / PARK_KP_DIV);
                    if (s > STEER_MAX)       s = STEER_MAX;
                    else if (s < -STEER_MAX) s = (int16_t)-STEER_MAX;

                    park_sub--;
                    SET_drive((int16_t)(LANE_SPEED + s), (int16_t)(LANE_SPEED - s));
                }
                else if (junction_now)
                {
                    lane_state = PARK_PAUSE;
                    park_sub = 0;
                    SET_stop();
                }
                else
                {
                    int16_t s = (int16_t)(err / PARK_KP_DIV);
                    if (s > STEER_MAX)       s = STEER_MAX;
                    else if (s < -STEER_MAX) s = (int16_t)-STEER_MAX;

                    SET_drive((int16_t)(LANE_SPEED + s), (int16_t)(LANE_SPEED - s));
                }
            }
            else if (lane_state == PARK_PAUSE)
            {

                SET_stop();

                if (park_sub < 65535)
                    park_sub++;

                if (park_sub >= PARK_PAUSE_TICKS)
                {
                    lane_state = PARK_ADV;
                    park_sub = 0;
                }
            }
            else if (lane_state == PARK_ADV)
            {

                SET_drive(LANE_SPEED, LANE_SPEED);

                if (park_sub < 65535)
                    park_sub++;

                if (park_sub >= PARK_ADV_GRACE
                    && (on_line || park_sub >= PARK_ADV_MAX))
                {
                    lane_state = PARK_SPIN;
                    park_sub = 0;
                    SET_stop();
                }
            }
            else if (lane_state == PARK_SPIN)
            {

                SET_drive((int16_t)(LANE_SPEED + STEER_MAX),
                          (int16_t)(LANE_SPEED - STEER_MAX));

                if (park_sub < 65535)
                    park_sub++;

                if (park_sub >= PARK_SPIN_TICKS)
                {
                    lane_state = PARK_WAIT;
                    park_sub = 0;
                }
            }
            else if (lane_state == PARK_WAIT)
            {

                SET_stop();

                if (park_sub < 65535)
                    park_sub++;

                if (park_sub >= PARK_WAIT_TICKS)
                {
                    lane_state = PARK_OUT;
                    park_sub = PARK_OUT_GRACE;
                }
            }
            else if (lane_state == PARK_OUT)
            {

                SET_drive(LANE_SPEED, LANE_SPEED);

                if (park_sub > 0)
                {
                    park_sub--;
                }
                else if (junction_now)
                {
                    lane_state = PARK_EXIT;
                    park_sub = 0;

                    if (cross_count < 63)
                        cross_count++;
                }
            }
            else if (lane_state == PARK_EXIT)
            {

                SET_drive((int16_t)(LANE_SPEED - STEER_MAX),
                          (int16_t)(LANE_SPEED + STEER_MAX));

                if (park_sub < 65535)
                    park_sub++;

                if (park_sub >= PARK_EXIT_TICKS)
                    LANE_handOff();
            }
                        else if (JUNCTION_STEP && junction_hit)
            {

                junction_hit = 0;
                junction_run = 0;
                SET_stop();
                state = ST_IDLE;
            }
            else if (kick_cnt > 0)
            {
                kick_cnt--;
                SET_drive(KICK_SPEED, KICK_SPEED);
            }

            else if (cross_hold > 0 || on_line)
            {
                int32_t out;
                int16_t mag;
                int16_t shaped;
                int16_t derr;

                lost_cnt = 0;

                lost_dir = (err >= 0) ? 1 : -1;

                mag = (err < 0) ? (int16_t)-err : err;
                shaped = (int16_t)(((int32_t)err * mag) / ERR_FULL);

                if (cross_hold > 0)
                {

                    out = ((int32_t)KP * shaped) / PID_SHIFT;
                    err_integ = 0;
                    d_filt = 0;
                    in_junction = 1;
                }
                else
                {

                    if (in_junction)
                    {
                        in_junction = 0;
                        shaped_prev = shaped;
                        d_filt = 0;
                    }

                    derr = (int16_t)(shaped - shaped_prev);
                    d_filt = (int16_t)(((int32_t)d_filt * D_FILTER + derr) / (D_FILTER + 1));

                    err_integ += shaped;

                    if (err_integ > I_LIMIT)
                        err_integ = I_LIMIT;
                    else if (err_integ < -I_LIMIT)
                        err_integ = -I_LIMIT;

                    out = ((int32_t)KP * shaped
                         + (int32_t)KI * err_integ
                         + (int32_t)KD * d_filt) / PID_SHIFT;
                }

                if (out > STEER_MAX)
                    out = STEER_MAX;
                else if (out < -STEER_MAX)
                    out = -STEER_MAX;

                steer = (int16_t)out;

                err_prev = err;
                shaped_prev = shaped;

                SET_drive((int16_t)(base + steer), (int16_t)(base - steer));
            }
            else
            {

                int16_t mag = (err_prev < 0) ? (int16_t)-err_prev : err_prev;
                uint16_t straight = (mag > LOST_TURN_ERR) ? 0 : LOST_STRAIGHT;

                lost_cnt++;

                if (lost_cnt < straight)
                {
                    SET_drive(base, base);
                }
                else if (lost_cnt < straight + LOST_SPIN)
                {
                    int16_t s = (int16_t)LOST_SPEED;

                    SET_drive((int16_t)(lost_dir > 0 ?  s : -s),
                              (int16_t)(lost_dir > 0 ? -s :  s));
                }
                else
                {
                    SET_stop();
                    state = ST_IDLE;
                }
            }
        }
        else
        {
            SET_stop();
        }

        UPDATE_leds(state, blink);

        if (state == ST_RUN && cross_count != lcd_shown_count)
        {
            lcd_shown_count = cross_count;
            LCD_putCount(cross_count);
        }

        if (lcd_dirty && state != ST_RUN)
        {
            lcd_dirty = 0;
            UPDATE_lcd(state, SPEED_TABLE[spd_step]);
        }
    }
}

void INIT_tick(void)
{
    TCCR0 = (1 << WGM01) | (1 << CS02) | (1 << CS01) | (1 << CS00);
    OCR0 = 77;
    TIMSK |= (1 << OCIE0);
}

ISR(TIMER0_COMP_vect)
{
    tick = 1;
}

void INIT_adc(void)
{
    DDRF = 0x00;
    PORTF = 0x00;

    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN)
           | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t READ_adc(uint8_t ch)
{
    ADMUX = (uint8_t)((1 << REFS0) | (ch & 0x1F));

    ADCSRA |= (1 << ADSC);

    while (ADCSRA & (1 << ADSC))
        ;

    return ADC;
}

void READ_sensors(void)
{
    for (uint8_t i = 0; i < SENSOR_N; i++)
    {
        uint8_t  idx = SENSOR_REVERSE ? (uint8_t)(SENSOR_N - 1 - i) : i;
        uint16_t raw = READ_adc((uint8_t)(ADC_BASE_CH + idx));
        int32_t  span = (int32_t)cal_max[idx] - (int32_t)cal_min[idx];
        int32_t  v;

        if (span < 20)
            span = 20;

        v = ((int32_t)raw - (int32_t)cal_min[idx]) * NORM_MAX / span;

        if (v < 0)
            v = 0;
        else if (v > NORM_MAX)
            v = NORM_MAX;

        norm[i] = (int16_t)v;
    }
}

uint8_t UPDATE_polarity(void)
{
    uint8_t bright = 0;
    uint8_t cand;

    for (uint8_t i = 0; i < SENSOR_N; i++)
    {
        if (norm[i] > BRIGHT_TH)
            bright++;
    }

    if (bright >= 4)
        cand = 1;
    else if (bright <= 2)
        cand = 0;
    else
        cand = bg_white;

    if (cand == bg_white)
    {
        pol_count = 0;
    }
    else if (++pol_count >= POLARITY_HOLD)
    {
        bg_white = cand;
        pol_count = 0;

        err_integ = 0;
        err_prev = 0;
        shaped_prev = 0;
        d_filt = 0;
    }

    return bg_white;
}

int16_t CONVERT_weightToError(uint8_t* on_line)
{
    int32_t sw = 0;
    int32_t swx = 0;
    uint8_t hot = 0;
    uint8_t left_hot = 0;
    uint8_t right_hot = 0;
    uint8_t groups = 0;
    uint8_t prev_hot = 0;

    for (uint8_t i = 0; i < SENSOR_N; i++)
    {
        int16_t w = bg_white ? (int16_t)(NORM_MAX - norm[i]) : norm[i];

        w = (int16_t)(w - LINE_FLOOR);

        if (w < 0)
            w = 0;

        weight[i] = w;
        sw += w;
        swx += (int32_t)w * POS[i];

        if (w > CROSS_W)
        {
            hot++;

            if (!prev_hot)
                groups++;

            prev_hot = 1;

            if (i <= 1)
                left_hot = 1;

            if (i >= SENSOR_N - 2)
                right_hot = 1;
        }
        else
        {
            prev_hot = 0;
        }
    }

    *on_line = (uint8_t)(sw >= LINE_MIN_SUM);

    if (groups >= 2
        || hot >= ((cross_count == 0) ? WIDE_N_FIRST : WIDE_N)
        || (cross_count == WIDE_T_IDX && hot >= WIDE_N_T && (left_hot != right_hot)))
    {
        if (junction_run < JUNCTION_PERSIST)
            junction_run++;
    }
    else if (junction_run > JUNCTION_DECAY)
    {
        junction_run = (uint8_t)(junction_run - JUNCTION_DECAY);
    }
    else
    {
        junction_run = 0;
    }

    junction_now = (uint8_t)(junction_run >= JUNCTION_PERSIST);

    if (junction_now)
    {
        cross_clear = 0;
    }
    else if (cross_clear < CROSS_CLEAR)
    {
        cross_clear++;

        if (cross_clear >= CROSS_CLEAR)
            cross_armed = 1;
    }

    if (cross_lock > 0)
        cross_lock--;

    if (start_ignore > 0)
        start_ignore--;

    if (lane_state != LANE_OFF && lane_ignore > 0)
    {

        if (lane_ignore > 0)
            lane_ignore--;

        junction_now = 0;
    }

    if (junction_now && cross_armed && start_ignore == 0
        && cross_lock == 0 && cross_hold == 0 && cross_delay == 0
        && lane_state == LANE_OFF)
    {
        uint8_t idx = cross_count;

        cross_armed = 0;
        cross_clear = 0;
        junction_hit = 1;

        if (cross_count < 63)
            cross_count++;

        if (idx < JUNCTION_MAX)
        {
            cross_err = JUNCTION_ERR[idx];

            cross_delay = JUNCTION_DELAY[idx];
            cross_pending = JUNCTION_HOLD[idx];
            lane_pending = JUNCTION_LANE[idx];
            park_pending = JUNCTION_PARK[idx];

            cross_align = JUNCTION_ALIGN[idx];
            cross_align_ok = 0;
        }
        else
        {

            if (right_hot && !left_hot)
                cross_err = CORNER_ERR;
            else if (left_hot && !right_hot)
                cross_err = (int16_t)-CORNER_ERR;
            else
                cross_err = err_prev;

            cross_delay = 0;
            cross_pending = 40;
            lane_pending = 0;
            cross_align = 0;
        }

        if (cross_delay == 0)
            cross_hold = cross_pending;
    }

    if (cross_delay > 0)
    {
        cross_delay--;

        if (cross_delay == 0)
            cross_hold = cross_pending;
    }

    if (cross_hold > 0)
    {

        if (cross_align > 0
            && (uint8_t)(cross_pending - cross_hold) >= cross_align)
        {
            int16_t c = (sw > 0) ? (int16_t)((swx * 100) / sw) : 0;
            int16_t m = (c < 0) ? (int16_t)-c : c;

            if (*on_line && hot <= LANE_EXIT_NARROW && m <= LANE_EXIT_CENTER)
            {
                if (cross_align_ok < 255)
                    cross_align_ok++;
            }
            else
            {
                cross_align_ok = 0;
            }

            if (cross_align_ok >= LANE_EXIT_HOLD)
                cross_hold = 1;
        }

        cross_hold--;

        if (cross_hold == 0)
        {
            cross_lock = CROSS_LOCK;

            cross_align = 0;

            if (lane_pending)
            {
                lane_pending = 0;
                lane_state = LANE_FWD;
                lane_sub = 0;
                lane_grace = LANE_GRACE;
                lane_ignore = LANE_IGNORE;
                lane_fwd_run = LANE_STUCK_RUN;
                lane_stuck = 0;
                lane_after_corner = 0;
                lane_arc_cleared = 0;
                lane_arc_run = 0;
                lane_contacts = 0;
                lane_probe = 0;
                lane_drift = 0;
            }
            else if (park_pending)
            {
                park_pending = 0;
                lane_state = PARK_IN;
                park_sub = PARK_IN_GRACE;
            }
        }

        return cross_err;
    }

    if (sw <= 0)
        return err_prev;

    if (hot >= CROSS_N && junction_run == 0)
    {
        if (right_hot && !left_hot)
            return CORNER_ERR;

        if (left_hot && !right_hot)
            return (int16_t)-CORNER_ERR;
    }

    return (int16_t)((swx * 100) / sw);
}

uint8_t CONVERT_spdToDuty(int16_t spd, uint8_t dmin, uint8_t dmax)
{
    if (spd < 0)
        spd = (int16_t)-spd;

    if (spd == 0)
        return 0;

    if (spd > 100)
        spd = 100;

    return (uint8_t)(dmin + ((uint16_t)(dmax - dmin) * (uint16_t)spd) / 100);
}

void SET_motorA(int16_t spd)
{
    int8_t  d = (spd > 0) ? (int8_t)1 : (spd < 0) ? (int8_t)-1 : (int8_t)0;
    uint8_t duty;

    if (cross_count >= MKICK_AFTER && d != 0 && d != dir_a)
        mkick_a = MKICK_TICKS;

    dir_a = d;

    duty = CONVERT_spdToDuty(spd, DUTY_MIN_A, DUTY_MAX_A);

    if (mkick_a > 0)
    {
        mkick_a--;

        if (duty > 0 && duty < MKICK_DUTY)
            duty = MKICK_DUTY;
    }

    if (duty == 0)
        PORTB &= ~((1 << IN1) | (1 << IN2));
    else if (spd > 0)
    {
        PORTB |= (1 << IN1);
        PORTB &= ~(1 << IN2);
    }
    else
    {
        PORTB &= ~(1 << IN1);
        PORTB |= (1 << IN2);
    }

    OCR1A = (uint16_t)(((uint32_t)PWM_TOP * duty) / 100);
}

void SET_motorB(int16_t spd)
{
    int8_t  d = (spd > 0) ? (int8_t)1 : (spd < 0) ? (int8_t)-1 : (int8_t)0;
    uint8_t duty;

    if (cross_count >= MKICK_AFTER && d != 0 && d != dir_b)
        mkick_b = MKICK_TICKS;

    dir_b = d;

    duty = CONVERT_spdToDuty(spd, DUTY_MIN_B, DUTY_MAX_B);

    if (mkick_b > 0)
    {
        mkick_b--;

        if (duty > 0 && duty < MKICK_DUTY)
            duty = MKICK_DUTY;
    }

    if (duty == 0)
        PORTB &= ~((1 << IN3) | (1 << IN4));
    else if (spd > 0)
    {
        PORTB |= (1 << IN3);
        PORTB &= ~(1 << IN4);
    }
    else
    {
        PORTB &= ~(1 << IN3);
        PORTB |= (1 << IN4);
    }

    OCR1B = (uint16_t)(((uint32_t)PWM_TOP * duty) / 100);
}

void SET_drive(int16_t left, int16_t right)
{
    if (left > 100)
        left = 100;
    else if (left < -100)
        left = -100;

    if (right > 100)
        right = 100;
    else if (right < -100)
        right = -100;

    if (LEFT_INVERT)
        left = (int16_t)-left;

    if (RIGHT_INVERT)
        right = (int16_t)-right;

    if (LEFT_IS_A)
    {
        SET_motorA(left);
        SET_motorB(right);
    }
    else
    {
        SET_motorA(right);
        SET_motorB(left);
    }
}

void SET_stop(void)
{
    SET_motorA(0);
    SET_motorB(0);
}

void INIT_motor(void)
{
    PORTB &= ~(IN_MASK | EN_MASK);
    DDRB |= (IN_MASK | EN_MASK);

    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);

    ICR1 = PWM_TOP;
    OCR1A = 0;
    OCR1B = 0;
}

void INIT_io(void)
{
    DDRA = 0xFF;
    PORTA = 0xFF;

    DDRC &= ~((1 << PC0) | (1 << PC1));
    PORTC |= (1 << PC0) | (1 << PC1);

    DDRD &= ~((1 << PD2) | (1 << PD3));
    PORTD |= (1 << PD2) | (1 << PD3);
}

uint8_t LOAD_calibration(void)
{
    if (eeprom_read_word(&ee_magic) == EE_MAGIC)
    {
        eeprom_read_block(cal_min, ee_min, sizeof(cal_min));
        eeprom_read_block(cal_max, ee_max, sizeof(cal_max));
        return 1;
    }

    for (uint8_t i = 0; i < SENSOR_N; i++)
    {
        cal_min[i] = 0;
        cal_max[i] = 1023;
    }

    return 0;
}

void SAVE_calibration(void)
{
    eeprom_write_block(cal_min, ee_min, sizeof(cal_min));
    eeprom_write_block(cal_max, ee_max, sizeof(cal_max));
    eeprom_write_word(&ee_magic, EE_MAGIC);
}

uint8_t READ_edge(uint8_t now, uint8_t* prev)
{
    uint8_t edge = (uint8_t)(now && !*prev);

    *prev = now;

    return edge;
}

static void PUT_line(uint8_t row, const char* src, uint8_t num, uint8_t show_num)
{
    char buf[17];
    uint8_t i = 0;

    while (src[i] && i < 16)
    {
        buf[i] = src[i];
        i++;
    }

    if (show_num && i <= 13)
    {
        buf[i++] = (char)('0' + (num / 100) % 10);
        buf[i++] = (char)('0' + (num / 10) % 10);
        buf[i++] = (char)('0' + num % 10);
    }

    while (i < 16)
        buf[i++] = ' ';

    buf[16] = '\0';

    lcdString(row, 0, buf);
}

static void LCD_putCount(uint8_t n)
{
    char b[3];

    b[0] = (char)('0' + (n / 10) % 10);
    b[1] = (char)('0' + n % 10);
    b[2] = '\0';

    lcdString(0, 5, b);
}

static void LCD_putPsd(uint16_t v)
{
    char b[5];

    if (v > 9999)
        v = 9999;

    b[0] = (char)('0' + (v / 1000) % 10);
    b[1] = (char)('0' + (v / 100) % 10);
    b[2] = (char)('0' + (v / 10) % 10);
    b[3] = (char)('0' + v % 10);
    b[4] = '\0';

    lcdString(1, 4, b);
}

static void RESET_psdLog(void)
{
    psd_head = 0;
    psd_n = 0;
    psd_div = PSD_LOG_EVERY;
    psd_lo = 255;
    psd_hi = 0;
    psd_wmax = 0;
}

static void SAMPLE_psd(void)
{
    uint16_t sum = 0;
    uint16_t lo = 1023;
    uint16_t hi = 0;
    uint16_t avg;
    uint16_t w;
    uint8_t  v;

    if (!psd_armed)
    {
        if (cross_count < PSD_LOG_AFTER)
            return;

        psd_armed = 1;
    }

    if (psd_div > 0)
    {
        psd_div--;
        return;
    }

    psd_div = PSD_LOG_EVERY;

    (void)READ_adc(PSD_CH);

    for (uint8_t i = 0; i < PSD_LOG_BURST; i++)
    {
        uint16_t r = READ_adc(PSD_CH);

        sum = (uint16_t)(sum + r);

        if (r < lo)
            lo = r;

        if (r > hi)
            hi = r;
    }

    avg = (uint16_t)(sum / PSD_LOG_BURST);
    psd_show = avg;
    psd_seen = 1;
    w = (uint16_t)(hi - lo);
    v = (uint8_t)(avg >> 2);

    psd_log[psd_head] = v;
    psd_head = (uint8_t)((psd_head + 1) % PSD_LOG_N);

    if (psd_n < PSD_LOG_N)
        psd_n++;

    if (v < psd_lo)
        psd_lo = v;

    if (v > psd_hi)
        psd_hi = v;

    if (w > psd_wmax)
        psd_wmax = (uint8_t)((w > 255) ? 255 : w);
}

static uint8_t IS_centerAligned(void)
{
    uint8_t mid = 0;

    for (uint8_t i = 0; i < SENSOR_N; i++)
    {
        uint8_t sees = (uint8_t)(weight[i] > STOP6_SEE);

        if (i == STOP6_MID_A || i == STOP6_MID_B)
        {
            if (sees)
                mid = 1;
        }
        else if (sees)
        {
            return 0;
        }
    }

    return mid;
}

static void LANE_handOff(void)
{
    lane_state = LANE_OFF;
    lane_ignore = 0;
    lane_grace = 0;
    err_integ = 0;
    in_junction = 1;
}

void UPDATE_lcd(uint8_t state, uint8_t spd)
{
    if (state == ST_CALIB)
    {
        PUT_line(0, "CALIBRATING", 0, 0);
        PUT_line(1, "SW2 = save", 0, 0);
        return;
    }

    if (show_page == PAGE_JCT)
    {
        PUT_line(0, "JUNCTIONS: ", cross_count, 1);
        PUT_line(1, "SW4 = next", 0, 0);
        return;
    }

    if (show_page == PAGE_PSD_SUM)
    {
        PUT_psdSummary();
        return;
    }

    if (show_page >= PAGE_PSD_0)
    {
        PUT_psdPage((uint8_t)(show_page - PAGE_PSD_0));
        return;
    }

    PUT_line(0, "READY  SW1=run", 0, 0);
    PUT_line(1, "SW4=pg  SPD=", spd, 1);
}

static void FMT_u8(char* dst, uint8_t v)
{
    dst[0] = (char)('0' + (v / 100) % 10);
    dst[1] = (char)('0' + (v / 10) % 10);
    dst[2] = (char)('0' + v % 10);
}

static void PUT_psdPage(uint8_t page)
{
    char line[2][17];
    uint8_t start = (uint8_t)((psd_head + PSD_LOG_N - psd_n) % PSD_LOG_N);

    for (uint8_t r = 0; r < 2; r++)
    {
        for (uint8_t i = 0; i < 16; i++)
            line[r][i] = ' ';

        line[r][16] = '\0';

        for (uint8_t k = 0; k < 4; k++)
        {
            uint8_t slot = (uint8_t)(page * 8 + r * 4 + k);
            char* dst = &line[r][1 + k * 4];

            if (slot < psd_n)
                FMT_u8(dst, psd_log[(start + slot) % PSD_LOG_N]);
            else
                dst[0] = dst[1] = dst[2] = '-';
        }
    }

    line[0][0] = (char)('A' + page);

    lcdString(0, 0, line[0]);
    lcdString(1, 0, line[1]);
}

static void PUT_psdSummary(void)
{
    char line[2][17];

    for (uint8_t r = 0; r < 2; r++)
    {
        for (uint8_t i = 0; i < 16; i++)
            line[r][i] = ' ';

        line[r][16] = '\0';
    }

    line[0][0] = 'P';
    line[0][1] = 'S';
    line[0][2] = 'D';
    line[0][4] = 'L';
    FMT_u8(&line[0][5], (uint8_t)((psd_n > 0) ? psd_lo : 0));
    line[0][9] = 'H';
    FMT_u8(&line[0][10], psd_hi);

    line[1][0] = 'W';
    FMT_u8(&line[1][1], psd_wmax);
    line[1][5] = 'N';
    FMT_u8(&line[1][6], psd_n);
    line[1][10] = 'C';
    FMT_u8(&line[1][11], cross_count);
    line[1][15] = psd_armed ? '*' : ' ';

    lcdString(0, 0, line[0]);
    lcdString(1, 0, line[1]);
}

void UPDATE_leds(uint8_t state, uint8_t blink)
{
    uint8_t mask = 0;

    if (state == ST_CALIB)
    {
        PORTA = blink ? 0x00 : 0xFF;
        return;
    }

    if (show_page == PAGE_JCT && state != ST_RUN)
    {
        PORTA = (uint8_t)~(cross_count & 0x3F);
        return;
    }

    if (show_page >= PAGE_PSD_SUM && state != ST_RUN)
    {
        PORTA = 0xFF;
        return;
    }

    for (uint8_t i = 0; i < SENSOR_N; i++)
    {
        if (weight[i] > LED_TH)
            mask |= (uint8_t)(1 << i);
    }

    if (junction_now)
        mask |= (1 << 6);

    if (state == ST_RUN)
        mask |= (1 << 7);

    PORTA = (uint8_t)~mask;
}
