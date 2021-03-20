#include <eason-s140.h>
#include <ble_bps.h>
#include <ble_bas.h>
#include <ble_dis.h>
#include <ble_advertising.h>
#include <sensorsim.h>
#include <peer_manager.h>

    #define NUM_BPS_SIM_MEAS 4
    static struct {
        ieee_float16_t systolic   ;
        ieee_float16_t diastolic  ;
        ieee_float16_t pulse_rate ;
        ieee_float16_t mean_arterial   ;
    }bps_sim_meas[NUM_BPS_SIM_MEAS] = { { 0 } };

static void bps_sim_measurement(ble_bps_meas_t *pMeas) {
        memset(pMeas,0,sizeof(ble_bps_meas_t));
        ble_date_time_t dataTime={2021,3,26,18,43,15};    
        static unsigned char index = 0;
    pMeas->blood_pressure_diastolic.mantissa =  bps_sim_meas[index].diastolic.mantissa ;
    pMeas->blood_pressure_systolic.mantissa = bps_sim_meas[index].systolic.mantissa ;
    pMeas->mean_arterial_pressure.mantissa = bps_sim_meas[index].mean_arterial.mantissa ;
    pMeas->pulse_rate.mantissa = bps_sim_meas[index].pulse_rate.mantissa;

    pMeas->blood_pressure_units_in_kpa = false ;
    pMeas->user_id_present = false;
    pMeas->measurement_status_present = false;
    // pMeas->measurement_status = ;
    pMeas->time_stamp = dataTime;
    pMeas->pulse_rate_present = (index==0) || (index==2);
    pMeas->time_stamp_present = (index==0) || (index==1) ;

    if(++index==NUM_BPS_SIM_MEAS)   index = 0;

    dataTime.seconds +=27;
    if(dataTime.seconds > 59) {
        dataTime.seconds -= 60;
        dataTime.minutes += 1;
        if(dataTime.minutes > 59) {
            dataTime.minutes = 0;
        }
    }
}

    static bool bps_meas_ind_pending = false ;
static void blood_pressure_measurement_send()   {
        bool is_indication_enabled ;
        ble_bps_meas_t bpsMeas = { 0 } ;
    ble_bps_is_indication_enabled(&m_bps,&is_indication_enabled) ;
    if(is_indication_enabled && !bps_meas_ind_pending) {
        bps_sim_measurement(&bpsMeas) ;
        switch ( ble_bps_measurement_send(&m_bps, &bpsMeas) ) {
            case NRF_SUCCESS :
                bps_meas_ind_pending = true;
                break;
            case NRF_ERROR_INVALID_STATE :
                break;
        }
    }
}

    APP_TIMER_DEF(m_timer_battery);
static void app_timers_init()   {
    app_timer_init();
    app_timer_create(&m_timer_battery,APP_TIMER_MODE_REPEATED,NULL);
}

    static void hndl_evt_ble() {

    }
/*01*/ static void ble_stack_init() {
    nrf_sdh_enable_request();
        unsigned int ram_start = 0;
    nrf_sdh_ble_default_cfg_set(1, &ram_start );
    nrf_sdh_ble_enable(&ram_start);
    NRF_SDH_BLE_OBSERVER(m_observer,3,hndl_evt_ble,NULL);
}

/*02*/ static void gap_params_init() {
        ble_gap_conn_sec_mode_t sec_mode = { 0 };
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    sd_ble_gap_device_name_set(&sec_mode, "Nordic_BPS",strlen("Nordic_BPS"));
    sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_BLOOD_PRESSURE);
        ble_gap_conn_params_t conn_params = { 0 };
        conn_params.conn_sup_timeout  = MSEC_TO_UNITS(4000, UNIT_10_MS);
        conn_params.max_conn_interval = MSEC_TO_UNITS(1000, UNIT_1_25_MS);
        conn_params.min_conn_interval = MSEC_TO_UNITS(500, UNIT_1_25_MS);
        conn_params.slave_latency  = 0 ;
    sd_ble_gap_ppcp_set(&conn_params);
}

    NRF_BLE_GATT_DEF(m_gatt);
/*03*/ static void gatt_init() {
    nrf_ble_gatt_init(&m_gatt, NULL);
}
    BLE_BPS_DEF(m_bps);
    BLE_BAS_DEF(m_bas);
    NRF_BLE_QWR_DEF(m_qwr);
    static void hndl_err_qwr() {  ;  }
    static void hndl_evt_bps(ble_bps_t * p_bps, ble_bps_evt_t * p_evt) {  
        switch(p_evt->evt_type){
            case BLE_BPS_EVT_INDICATION_ENABLED :
                blood_pressure_measurement_send();
                break;
            case BLE_BPS_EVT_INDICATION_CONFIRMED :
                bps_meas_ind_pending = false ;
                break;
            default :
                break;
        }
    }
/*04*/ static void services_init() {
        nrf_ble_qwr_init_t qwr_init = { .error_handler = hndl_err_qwr } ;
        ble_bps_init_t bps_init = { 0 };
            bps_init.bp_cccd_wr_sec = SEC_JUST_WORKS ;
            bps_init.bp_feature_rd_sec = SEC_OPEN ;
            bps_init.evt_handler = hndl_evt_bps ;
            bps_init.feature = BLE_BPS_FEATURE_BODY_MOVEMENT_BIT | BLE_BPS_FEATURE_MEASUREMENT_POSITION_BIT ;
        ble_bas_init_t bas_init = { 0 };
            bas_init.bl_cccd_wr_sec = SEC_OPEN ;
            bas_init.bl_rd_sec = SEC_OPEN ;
            bas_init.bl_report_rd_sec = SEC_OPEN ;
            bas_init.evt_handler = NULL ;
            bas_init.p_report_ref = NULL;
            bas_init.support_notification = true ;
        ble_dis_init_t dis_init = { 0 };
            ble_srv_ascii_to_utf8(&dis_init.manufact_name_str,"NordicSemiconductor");
            ble_srv_ascii_to_utf8(&dis_init.model_num_str, "Nordic-BPS-Example");
        ble_dis_sys_id_t sys_id = { 0 };
            sys_id.manufacturer_id = 0x1122334455;
            sys_id.organizationally_unique_id = 0x667788;
            dis_init.p_sys_id = &sys_id ;

    nrf_ble_qwr_init(&m_qwr, &qwr_init);
    ble_bps_init(&m_bps, &bps_init);
    ble_bas_init(&m_bas, &bas_init);
    ble_dis_init(&dis_init);
}

    BLE_ADVERTISING_DEF(m_advertising);
    ble_uuid_t adv_data[] = {
        {BLE_UUID_BLOOD_PRESSURE_SERVICE, BLE_UUID_TYPE_BLE},
        {BLE_UUID_BATTERY_SERVICE,BLE_UUID_TYPE_BLE},
        {BLE_UUID_DEVICE_INFORMATION_SERVICE,BLE_UUID_TYPE_BLE}
    };
    static void hndl_evt_advertising() {
    }
/*05*/ static void advertising_init() {
        ble_advertising_init_t adv_init = { 0 };
        adv_init.advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
        adv_init.advdata.include_appearance = true ;
        adv_init.advdata.name_type = BLE_ADVDATA_FULL_NAME ;
        adv_init.config.ble_adv_fast_enabled = true ;
        adv_init.config.ble_adv_fast_interval = 40 ;
        adv_init.config.ble_adv_fast_timeout = 18000 ;
        adv_init.error_handler = NULL ;
        adv_init.evt_handler = hndl_evt_advertising;
        adv_init.srdata.uuids_complete.p_uuids = adv_data;
        adv_init.srdata.uuids_complete.uuid_cnt = sizeof(adv_data)/sizeof(adv_data[0]);
    ble_advertising_init(&m_advertising, &adv_init);
    ble_advertising_conn_cfg_tag_set(&m_advertising, 1);
}

static void sensor_simulator_init() {
        static sensorsim_state_t sensim_state = { 0 };
        static sensorsim_cfg_t  sensim_cfg = { 0 };
            sensim_cfg.start_at_max = false ;
            sensim_cfg.min = 81 ;
            sensim_cfg.min = 100 ;
            sensim_cfg.incr = 1 ;
    sensorsim_init(&sensim_state , &sensim_cfg);

        bps_sim_meas[0].diastolic.mantissa      = DIASTOLIC_0 ;
        bps_sim_meas[0].mean_arterial.mantissa  = MEAN_ARTERIAL_0 ;
        bps_sim_meas[0].pulse_rate.mantissa     = PULSE_RATE_0 ;
        bps_sim_meas[0].systolic.mantissa       = SYSTOLIC_0 ;
        bps_sim_meas[1].diastolic.mantissa      = DIASTOLIC_1 ;
        bps_sim_meas[1].mean_arterial.mantissa  = MEAN_ARTERIAL_1 ;
        bps_sim_meas[1].pulse_rate.mantissa     = PULSE_RATE_1 ;
        bps_sim_meas[1].systolic.mantissa       = SYSTOLIC_1 ;
        bps_sim_meas[2].diastolic.mantissa      = DIASTOLIC_2 ;
        bps_sim_meas[2].mean_arterial.mantissa  = MEAN_ARTERIAL_2 ;
        bps_sim_meas[2].pulse_rate.mantissa     = PULSE_RATE_2 ;
        bps_sim_meas[2].systolic.mantissa       = SYSTOLIC_2 ;
        bps_sim_meas[3].diastolic.mantissa      = DIASTOLIC_3 ;
        bps_sim_meas[3].mean_arterial.mantissa  = MEAN_ARTERIAL_3 ;
        bps_sim_meas[3].pulse_rate.mantissa     = PULSE_RATE_3 ;
        bps_sim_meas[3].systolic.mantissa       = SYSTOLIC_3 ;
}

    static void hndl_evt_conn_params() {
    }
    static void hndl_err_conn_params() {
    }
/*06*/ static void conn_params_init() {
        ble_conn_params_init_t cp_init = { 0 };
        cp_init.disconnect_on_fail = false ;
        cp_init.error_handler = hndl_err_conn_params;
        cp_init.evt_handler = hndl_evt_conn_params;
        cp_init.max_conn_params_update_count = 3;
        cp_init.first_conn_params_update_delay = APP_TIMER_TICKS(30000);
        cp_init.next_conn_params_update_delay = APP_TIMER_TICKS(5000);
        cp_init.p_conn_params = NULL;
        cp_init.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID ;
    ble_conn_params_init(&cp_init);
}

    static void hndl_evt_pm(pm_evt_t const * pEvt) {
            bool is_indication_enabled ;
        switch(pEvt->evt_id) {
            case PM_EVT_PEERS_DELETE_SUCCEEDED:
                advertising_start(false);
                break;
            case PM_EVT_CONN_SEC_SUCCEEDED:
                ble_bps_is_indication_enabled(&m_bps,&is_indication_enabled);
                if(is_indication_enabled)   blood_pressure_measurement_send();
                break;
            defalut :
                break;
        }
    }
static void peer_manager_init()  {
    pm_init();
        ble_gap_sec_params_t sec_params = { 0 } ;
        sec_params.bond = 1;
        sec_params.io_caps = BLE_GAP_IO_CAPS_NONE ;
        sec_params.kdist_own.enc = 1;
        sec_params.kdist_own.id = 1;
        sec_params.kdist_peer.enc = 1;
        sec_params.kdist_peer.id = 1;
        sec_params.keypress = 0 ;
        sec_params.lesc = 0 ;
        sec_params.max_key_size = 16 ;
        sec_params.min_key_size = 7 ;
        sec_params.mitm = 0 ;
        sec_params.oob = 0 ;
    pm_sec_params_set(&sec_params);
    pm_register(hndl_evt_pm);
}

static void application_timers_start() {
    app_timer_start(&m_timer_battery,1000,NULL);
}

/*07*/ static void advertising_start(bool bSuccess) {
    ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST)
}
