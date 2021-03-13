#include <eason-s140.h>

    static void hndl_evt_ble(...)   {  ; }
    #define APP_BLE_CONN_CFG_TAG        1
    #define APP_BLE_OBSERVER_PRIO       3
/*01*/static void ble_stack_init()     {   //======================================<< 
    ACEE(nrf_sdh_enable_request()); 
        uint32_t ram_start=0;
    ACEE( nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start )); 
    ACEE( nrf_sdh_ble_enable(&ram_start) ) ; 
    ACEE( NRF_SDH_BLE_OBSERVER(m_observer,APP_BLE_OBSERVER_PRIO,hndl_evt_ble,NULL) );  
}

    #define DEVICE_NAME                 "Nordic_Blinky"
    #define CONN_SUP_TIMEOUT            MSEC_TO_UNITS(4000,UNIT_10_MS)      //4s
    #define MAX_CONN_INTERVAL           MSEC_TO_UNITS(200,UNIT_1_25_MS)     //0.2s
    #define MIN_CONN_INTERVAL           MSEC_TO_UNITS(100,UNIT_1_25_MS)     //0.1s
    #define SLAVE_LATENCY               0
/*02*/static void gap_params_init()     {   //======================================<< 
        ble_gap_conn_sec_mode_t sec_mode={0};
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode); 
    ACEE( sd_ble_gap_device_name_set(&sec_mode,DEVICE_NAME, strlen(DEVICE_NAME)) ); 

        ble_gap_conn_params_t conn_params={0};
        conn_params.conn_sup_timeout = CONN_SUP_TIMEOUT;
        conn_params.max_conn_interval = MAX_CONN_INTERVAL;
        conn_params.min_conn_interval = MIN_CONN_INTERVAL;
        conn_params.slave_latency = SLAVE_LATENCY;
    ACEE( sd_ble_gap_ppcp_set(&conn_params) );  
}
    NRF_BLE_GATT_DEF(m_gatt);
/*03*/static void gatt_init()     {         //======================================<< 
    nrf_ble_gatt_init(&m_gatt,NULL);   
}

    static void hndl_err_qwr(...)   {  ; }
    static void hndl_svc_lbs(...)   {  ; }
    NRF_BLE_QWR_DEF(m_qwr);
    BLE_LBS_DEF(m_lbs);
/*04*/static void services_init()     {     //======================================<< 
        nrf_ble_qwr_init_t qwr_init ;
        qwr_init.error_handler = hndl_err_qwr ;
    ACEE( nrf_ble_qwr_init(&m_qwr, &qwr_init) ); 
        ble_lbs_init_t lbs_init;
        lbs_init.led_write_handler = hndl_svc_lbs;
    ACEE( ble_lbs_init(&m_lbs, &lbs_init) );  
}

    #define APP_BLE_ADV_INTERVAL    64
    #define APP_BLE_ADV_DURATION    BLE_GAP_SCAN_TIMEOUT_UNLIMITED
    static uint8_t m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;
    static uint8_t m_enc_advdata[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
    static uint8_t m_enc_scan_rsp_data[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
    ble_gap_adv_data_t m_advdata = {
        .advdata={
            .p_data = m_enc_advdata ,
            .len = BLE_GAP_ADV_SET_DATA_SIZE_MAX 
        },
        .scan_rsp_data={
            .p_data = m_enc_scan_rsp_data ,
            .len = BLE_GAP_ADV_SET_DATA_SIZE_MAX
        }
    };
/*05*/static void advertising_init()     {      //======================================<< 
        ble_advdata_t advdata={0};
        advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
        advdata.include_appearance = true;
        advdata.name_type = BLE_ADVDATA_FULL_NAME;
    ACEE( ble_advdata_encode(&advdata, m_advdata.adv_data.p_data, m_advdata.adv_data.len) ); 
        ble_advdata_t srdata={0};
        ble_uuid_t adv_uuids[]={    { LBS_UUID_SERVICE, m_lbs.uuid_type }  };
        srdata.uuids_complete.p_uuids = adv_uuids;
        srdata.uuids_complete.uuid_cnt = sizeof(adv_uuids)/sizeof(adv_uuids[0]);
    ACEE( ble_advdata_encode(&srdata, m_advdata.scan_rsp_data.p_data, m_advdata.scan_rsp_data.len ) ); 
        ble_gap_adv_params_t adv_params={0};
        adv_params.duration = APP_BLE_ADV_DURATION;
        adv_params.filter_policy = BLE_GAP_ADV_FP_ANY;
        adv_params.interval = APP_BLE_ADV_INTERVAL;
        adv_params.p_peer_addr = NULL ;
        adv_params.primary_phy = BLE_GAP_PHY_1MBPS;
        adv_params.properties.type = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
    ACEE( sd_ble_gap_adv_set_configure(&m_adv_handle, &m_advdata, &adv_params) );  
}

    static void hndl_evt_conn_params(...)   {  ; }
    static void hndl_err_conn_params(...)   {  ; }
    #define MAX_CONN_PARAMS_UPDATE_COUNT    3
    #define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(20000)
    #define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000)
/*06*/static void conn_params_init()     {      //======================================<< 
        ble_conn_params_init_t cp_init={0};
        cp_init.disconnect_on_fail = false;
        cp_init.error_handler = hndl_err_conn_params;
        cp_init.evt_handler = hndl_evt_conn_params;
        cp_init.max_conn_params_update_count = MAX_CONN_PARAMS_UPDATE_COUNT;
        cp_init.p_conn_params = NULL ;
        cp_init.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID ;
        cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
        cp_init.next_conn_params_update_delay = NEXT_CONN_PARAMS_UPDATE_DELAY;
    ACEE( ble_conn_params_init(&cp_init) );   
}

/*07*/static void advertising_start()     {     //======================================<< 
    ACEE( sd_ble_gap_adv_start(&m_adv_handle,APP_BLE_CONN_CFG_TAG) );   
}

