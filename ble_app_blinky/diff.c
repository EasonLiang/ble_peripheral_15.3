
#define ADVERTISING_LED                 BSP_BOARD_LED_0                         /**< Is on when device is advertising. */
#define CONNECTED_LED                   BSP_BOARD_LED_1                         /**< Is on when device has connected. */
#define LEDBUTTON_LED                   BSP_BOARD_LED_2                         /**< LED to be toggled with the help of the LED Button Service. */
#define LEDBUTTON_BUTTON                BSP_BUTTON_0                            /**< Button that will trigger the notification event with the LED Button Service */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(20000)                  /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (15 seconds). */

#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50)                     /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */
#define DEAD_BEEF                       0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    switch (pin_no) {
        case LEDBUTTON_BUTTON:
            NRF_LOG_INFO("Send button state change.");
            err_code = ble_lbs_on_button_change(m_conn_handle, &m_lbs, button_action);
            if (err_code != NRF_SUCCESS             && err_code != BLE_ERROR_INVALID_CONN_HANDLE &&
                err_code != NRF_ERROR_INVALID_STATE && err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
                APP_ERROR_CHECK(err_code);
            break;
        default:
            APP_ERROR_HANDLER(pin_no);
            break;
    }
}
static void power_management_init(void)     {    ACEE (nrf_pwr_mgmt_init());  }
static void buttons_init(void){
    static app_button_cfg_t buttons[] =     {  {LEDBUTTON_BUTTON, false, BUTTON_PULL, button_event_handler} }; //The array must be static because a pointer to it will be saved in the button handler module.
    ACEE ( app_button_init(buttons, ARRAY_SIZE(buttons), BUTTON_DETECTION_DELAY) );
}
static void timers_init(void)   {    ACEE( app_timer_init() );   }  // Initialize timer module, making it use the scheduler
static void leds_init(void)     {    bsp_board_init(BSP_INIT_LEDS);     }
static void log_init(void){
    ACEE ( NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)    {    app_error_handler(DEAD_BEEF, line_num, p_file_name);   }

static void led_write_handler(uint16_t conn_handle, ble_lbs_t * p_lbs, uint8_t led_state){
    if (led_state){
        bsp_board_led_on(LEDBUTTON_LED);
        NRF_LOG_INFO("Received LED ON!");
    }else{
        bsp_board_led_off(LEDBUTTON_LED);
        NRF_LOG_INFO("Received LED OFF!");
    }
}
static void nrf_qwr_error_handler(uint32_t nrf_error)   {    APP_ERROR_HANDLER(nrf_error);      }
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)   { if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED) ACEE( sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE) ); }
static void conn_params_error_handler(uint32_t nrf_error)   { APP_ERROR_HANDLER(nrf_error); } //Function for handling a Connection Parameters error.
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{   
    switch (p_ble_evt->header.evt_id){
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected");
            bsp_board_led_on(CONNECTED_LED);    bsp_board_led_off(ADVERTISING_LED);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            ACEE(   nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle) );
            ACEE(   app_button_enable() );
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected");
            bsp_board_led_off(CONNECTED_LED);
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            ACEE( app_button_disable() );
            advertising_start();
            break;
        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:            // Pairing not supported            
            ACEE( sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL) );
            break;
        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys ={
                .rx_phys = BLE_GAP_PHY_AUTO,   .tx_phys = BLE_GAP_PHY_AUTO,
            };
            ACEE( sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys) );
            break;
        case BLE_GATTS_EVT_SYS_ATTR_MISSING:    // No system attributes have been stored.            
            ACEE( sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0) );
            break;
        case BLE_GATTC_EVT_TIMEOUT:             // Disconnect on GATT Client timeout event.            
            NRF_LOG_DEBUG("GATT Client Timeout.");
            ACEE( sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION) );
            break;
        case BLE_GATTS_EVT_TIMEOUT:             // Disconnect on GATT Server timeout event.            
            NRF_LOG_DEBUG("GATT Server Timeout.");
            ACEE( sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION) );
            break;
        default:    // No implementation needed.            
            break;
    }
}

static void idle_state_handle(void)         {    if (NRF_LOG_PROCESS() == false)  nrf_pwr_mgmt_run();   }
int main(void)
{
    log_init();     leds_init();      timers_init();   buttons_init();    power_management_init();
    ble_stack_init();    gap_params_init();   gatt_init();    services_init();   advertising_init();   conn_params_init();   advertising_start();
    NRF_LOG_INFO("Blinky example started.");
    for (;;)        idle_state_handle();
}