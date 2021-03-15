#include <eason-s140.h>
#include <ble_nus.h>
#include <ble_advertising.h>

BLE_ADVERTISING_DEF(m_advertising);
static void advertising_start(){
    ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
}

    static void hndl_evt_conn_params(...)   { ; };
    static void hndl_err_conn_params(...)   { ; };
static void conn_params_init(){
        ble_conn_params_init_t cp_init;
        cp_init.disconnect_on_fail = false ;
        cp_init.error_handler = hndl_evt_conn_params ;
        cp_init.evt_handler = hndl_evt_conn_params;
        cp_init.max_conn_params_update_count = 3 ;
        cp_init.p_conn_params = NULL ;
        cp_init.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID;
        cp_init.first_conn_params_update_delay = APP_TIMER_TICKS(30000);
        cp_init.next_conn_params_update_delay = APP_TIMER_TICKS(5000);
    ble_conn_params_init(&cp_init);
}

    static ble_uuid_t m_adv_uuids[]={  {   BLE_UUID_NUS_SERVICE,  BLE_UUID_TYPE_VENDOR_BEGIN }};
    static void hndl_evt_advertising(...) { ; };
static void advertising_init(){
        ble_advertising_init_t init;
        init.advdata.flags =BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
        init.advdata.include_appearance = false;
        init.advdata.name_type = BLE_ADVDATA_FULL_NAME;
        init.srdata.uuids_complete.p_uuids = m_adv_uuids;
        init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids)/sizeof(m_adv_uuids[0]);
        init.config.ble_adv_fast_enabled = true;
        init.config.ble_adv_fast_interval = 64;
        init.config.ble_adv_fast_timeout = 18000 ;
        init.evt_handler = hndl_evt_advertising;
    ble_advertising_init(&m_advertising, &init);
    ble_advertising_conn_cfg_tag_set(&m_advertising, 1);
}

    NRF_BLE_QWR_DEF(m_qwr);
    BLE_NUS_DEF(m_nus);
    static void hndl_err_qwr(...) { ; };
    static void hndl_svc_nus(...) { ; };
static void services_init(){
        nrf_ble_qwr_init_t qwr_init = {.error_handler = hndl_err_qwr  };
    nrf_ble_qwr_init(&m_qwr, &qwr_init);
        ble_nus_init_t nus_init = {.data_handler = hndl_svc_nus };
    ble_nus_init(&m_nus,&nus_init);
}

    NRF_BLE_GATT_DEF(m_gatt);
    static hndl_evt_gatt(...) {};
static void gatt_init(){
    nrf_ble_gatt_init(&m_gatt,hndl_evt_gatt);
    nrf_ble_gatt_att_mtu_periph_set(&m_gatt , NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
}

static void gap_params_init(){
        ble_gap_conn_sec_mode_t sec_mode={0};
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    ACEE( sd_ble_gap_device_name_set(&sec_mode,"Nordic_UART",strlen("Nordic_UART")) );
        ble_gap_conn_params_t conn_params={0};
        conn_params.conn_sup_timeout =  MSEC_TO_UNITS(4000,UNIT_10_MS);
        conn_params.max_conn_interval = MSEC_TO_UNITS(75,UNIT_1_25_MS);
        conn_params.min_conn_interval = MSEC_TO_UNITS(20,UNIT_1_25_MS);
        conn_params.slave_latency = 0 ;
    ACEE ( sd_ble_gap_ppcp_set(&conn_params) );
}

    static void hndl_evt_ble(...)   {   ;   };
static void ble_stack_init(){
    nrf_sdh_enable_request();
        uint32_t ram_start = 0 ;
    ACEE ( nrf_sdh_ble_default_cfg_set(1, &ram_start ) );
    ACEE ( nrf_sdh_ble_enable(&ram_start) );
    NRF_SDH_BLE_OBSERVER(m_observer,3,hndl_evt_ble,NULL);
}