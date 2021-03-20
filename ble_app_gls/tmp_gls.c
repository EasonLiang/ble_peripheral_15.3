#include <eason-s140.h>
#include <ble_gls.h>
#include <ble_bas.h>
#include <ble_dis.h>
#include <sensorsim.h>
#include <peer_manager.h>
#include <ble_advertising.h>

    APP_TIMER_DEF(m_timer_batt);
static void app_timers_init() {
    app_timer_init();
    app_timer_create(&m_timer_batt,APP_TIMER_MODE_REPEATED,NULL);
}

    static hndl_evt_ble() {     ;   };
/*01*/ static void ble_stack_init() {
    nrf_sdh_enable_request();
        unsigned int ram_start = 0 ;
    nrf_sdh_ble_default_cfg_set(1,&ram_start);
    nrf_sdh_ble_enable(&ram_start);
    NRF_SDH_BLE_OBSERVER(&m_observer, 3, hndl_evt_ble, NULL);
}

/*02*/ static void gap_params_init() {
        ble_gap_conn_sec_mode_t sec_mode = { 0 };
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    sd_ble_gap_device_name_set(&sec_mode,"Nordic_Glucose",strlen("Nordic_Glucose"));
    sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_GLUCOSE_METER);
        ble_gap_conn_params_t conn_params = { 0 } ;
        conn_params.conn_sup_timeout = CONN_SUP_TIMEOUT_GLS ;
        conn_params.max_conn_interval = MAX_CONN_INTERVAL_GLS ;
        conn_params.min_conn_interval = MIN_CONN_INTERVAL_GLS ;
        conn_params.slave_latency = SLAVE_LATENCY_GLS ;
    sd_ble_gap_ppcp_set(&conn_params);
}
    NRF_BLE_GATT_DEF(m_gatt);
/*03*/ static void gatt_init() {
    nrf_ble_gatt_init(&m_gatt,NULL);
}

    static void hndl_err_qwr() {    ;   }
    static void hndl_err_gls()  {   ;   }
    NRF_BLE_QWR_DEF(m_qwr);
    BLE_GLS_DEF(m_gls);
    BLE_BAS_DEF(m_bas);
/*04*/ static void services_init() {
        nrf_ble_qwr_init_t qwr_init = { .error_handler = hndl_err_qwr };
        ble_gls_init_t gls_init = { 0 } ;
        gls_init.error_handler = hndl_err_gls ;
        gls_init.evt_handler = NULL ;
        gls_init.feature = BLE_GLS_FEATURE_RES_HIGH_LOW | BLE_GLS_FEATURE_RES_HIGH_LOW | BLE_GLS_FEATURE_LOW_BATT;
        gls_init.gl_feature_rd_sec = SEC_OPEN ;
        gls_init.gl_meas_cccd_wr_sec = SEC_JUST_WORKS;
        gls_init.is_context_supported = NULL;
        gls_init.racp_cccd_wr_sec = SEC_JUST_WORKS ;
        gls_init.racp_wr_sec = SEC_JUST_WORKS;
        ble_bas_init_t bas_init = { 0 } ;
        bas_init.bl_cccd_wr_sec = SEC_OPEN;
        bas_init.bl_rd_sec = SEC_OPEN ;
        bas_init.bl_report_rd_sec = SEC_OPEN;
        bas_init.evt_handler = NULL ;
        bas_init.p_report_ref = NULL ;
        bas_init.support_notification = true ;
        bas_init.initial_batt_level = 100;
        ble_dis_init_t dis_init = { 0 } ;
        ble_srv_ascii_to_utf8(&dis_init.manufact_name_str,"NordicSemiconductor");
        ble_srv_ascii_to_utf8(&dis_init.model_num_str,"Nordic-GLS-Example");
        ble_dis_sys_id_t sys_id = { 0 };
        sys_id.manufacturer_id = 0x55AA55AA55;
        sys_id.organizationally_unique_id = 0x667788;
        dis_init.p_sys_id = & sys_id;

    nrf_ble_qwr_init(&m_qwr, &qwr_init);
    ble_gls_init(&m_gls, &gls_init);
    ble_bas_init(&m_bas,&bas_init);
    ble_dis_init(&dis_init);
}

    BLE_ADVERTISING_DEF(m_advertising);
    static void hndl_evt_advertising() {    ;   }
    ble_uuid_t adv_uuid[] = {
        {BLE_UUID_GLUCOSE_SERVICE,BLE_UUID_TYPE_BLE},
        {BLE_UUID_BATTERY_SERVICE,BLE_UUID_TYPE_BLE},
        {BLE_UUID_DEVICE_INFORMATION_SERVICE,BLE_UUID_TYPE_BLE}
    }
/*05*/ static void advertising_init() {
        ble_advertising_init_t init = { 0 };
        init.advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
        init.advdata.include_appearance = true ; 
        init.advdata.name_type = BLE_ADVDATA_FULL_NAME;
        init.config.ble_adv_fast_enabled = true;
        init.config.ble_adv_fast_interval = BLE_ADV_FAST_TIMEOUT_GLS;
        init.config.ble_adv_fast_timeout = BLE_ADV_FAST_TIMEOUT_GLS ;
        init.error_handler = NULL ;
        init.evt_handler = hndl_evt_advertising;
        init.srdata.uuids_complete.p_uuids = adv_uuid;
        init.srdata.uuids_complete.uuid_cnt = sizeof(adv_uuid)/sizeof(adv_uuid[0]);
    ble_advertising_init(&m_advertising, &init);
    ble_advertising_conn_cfg_tag_set(&m_advertising,1);
}

    static sensorsim_state_t ss_state = { 0 };
    static sensorsim_cfg_t   sensorsim_cfg = { 0 } ;
static void sensor_simulator_init() {
        sensorsim_cfg.start_at_max = true;
        sensorsim_cfg.min = 81;
        sensorsim_cfg.max = 100;
        sensorsim_cfg.incr = 1;
    sensorsim_init(&ss_state, &sensorsim_cfg);
}

    static void hndl_evt_conn_params() {    ;   }
    static void hndl_err_conn_params() {    ;   }
/*06*/ static void conn_params_init() {
        ble_conn_params_init_t cp_init = { 0 };
        cp_init.disconnect_on_fail = false ;
        cp_init.error_handler = hndl_err_conn_params;
        cp_init.evt_handler = hndl_evt_conn_params;
        cp_init.max_conn_params_update_count = 3;
        cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY_GLS ;
        cp_init.next_conn_params_update_delay = NEXT_CONN_PARAMS_UPDATE_DELAY_GLS ;
        cp_init.p_conn_params = NULL;
        cp_init.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID ;
    ble_conn_params_init(&cp_init);
}

    static void hndl_evt_pm(pm_evt_t *pmEvt) {
        switch(pmEvt->evt_id) {
            case PM_EVT_CONN_SEC_SUCCEEDED :
                break;
            case PM_EVT_PEERS_DELETE_SUCCEEDED :
                break;
            default :
                break;
        }
    }
static void peer_manager_init() {
    pm_init();
        ble_gap_sec_params_t sec_params = { 0 };
        sec_params.bond = 1;
        sec_params.io_caps = BLE_GAP_IO_CAPS_DISPLAY_ONLY ;
        sec_params.kdist_own.enc = 1;
        sec_params.kdist_own.id = 1;
        sec_params.kdist_peer.enc = 1;
        sec_params.kdist_peer.id = 1;
        sec_params.keypress = 0 ;
        sec_params.lesc = 0 ;
        sec_params.max_key_size = 16;
        sec_params.min_key_size = 7 ;
        sec_params.mitm = 0;
        sec_params.oob = 0;
    pm_sec_params_set(&sec_params);
    pm_register(hndl_evt_pm);
}

static void application_timers_start() {
    app_timer_start(m_timer_batt,APP_TIMER_TICKS(2000),NULL);
}

/*07*/ static void advertising_start() {
    ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
}

