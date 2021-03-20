#include <eason-s140.h>
#include <ble_advertising.h>
#include <ble_bps.h>        //blood pressure 
#include <ble_bas.h>        //battery
#include <ble_dis.h>        //device information
#include <peer_manager.h>   //
#include <sensorsim.h>

#define NUM_MEAS_SIM_VALUES     4
static struct {
    ieee_float16_t systolic         ;
    ieee_float16_t diastolic        ;
    ieee_float16_t mean_arterial    ;
    ieee_float16_t pulse_rate       ;
} bps_meas_sim_val[NUM_MEAS_SIM_VALUES] = { {0} };

static void bps_sim_measurement(ble_bps_meas_t *pMeas) {
    static ble_date_time_t s_time_stamp = { 2012,12,5, 11,05,03};
    typedef unsigned char uint8_t ;
    static uint8_t index = 0 ;

    pMeas->blood_pressure_diastolic.exponent = bps_meas_sim_val[index].diastolic.exponent ;
    pMeas->blood_pressure_diastolic.mantissa = bps_meas_sim_val[index].diastolic.mantissa ;
    pMeas->blood_pressure_systolic.exponent = bps_meas_sim_val[index].systolic.exponent ;
    pMeas->blood_pressure_systolic.mantissa = bps_meas_sim_val[index].systolic.mantissa ;
    pMeas->mean_arterial_pressure.exponent = bps_meas_sim_val[index].mean_arterial.exponent ;
    pMeas->mean_arterial_pressure.mantissa = bps_meas_sim_val[index].mean_arterial.mantissa ;
    // pMeas->measurement_status = false ;
    pMeas->pulse_rate.exponent = bps_meas_sim_val[index].pulse_rate.exponent ;
    pMeas->pulse_rate.mantissa = bps_meas_sim_val[index].pulse_rate.mantissa ;

    pMeas->time_stamp = s_time_stamp;
    
    pMeas->blood_pressure_units_in_kpa = false ;
    pMeas->time_stamp_present = (index==0)||(index==2) ;
    pMeas->pulse_rate_present = (index==0)||(index==1) ;
    pMeas->user_id_present = false ; 
    pMeas->measurement_status_present = false ;

    if(++index==NUM_MEAS_SIM_VALUES)  index = 0 ;
    s_time_stamp.seconds += 27 ;
    if(s_time_stamp.seconds > 59){
        s_time_stamp.seconds -= 60 ;
        s_time_stamp.minutes += 1 ;
        if(s_time_stamp.minutes > 59){
            s_time_stamp.minutes -= 60 ;
        }
    }
}
    static bool m_bps_meas_ind_conf_pending = false ; 
static void blood_pressure_measurement_send()   {
    ble_bps_meas_t simulated_meas = { 0 } ;
    unsigned int err_code ;
    bool is_indication_enabled ;
    ACEE ( ble_bps_is_indication_enabled(&m_bps,&is_indication_enabled) );

    if( is_indication_enabled && !m_bps_meas_ind_conf_pending ) {
        bps_sim_measurement(&simulated_meas);
        err_code = ble_bps_measurement_send(&m_bps, &simulated_meas) ;
        switch(err_code){
            case NRF_SUCCESS :
                m_bps_meas_ind_conf_pending = true ;
                break;
            case NRF_ERROR_INVALID_STATE :
                break;
            default :
                APP_ERROR_HANDLER(err_code);
                break;
        }
    }
}

    BLE_ADVERTISING_DEF(m_advertising);
static void advertising_start(bool erase_bond) {
    ble_advertising_start(&m_advertising,BLE_ADV_MODE_FAST);
}

    static hndl_evt_pm( pm_evt_t * p_evt)    {   
        bool is_indication_enabled ; 
        switch(p_evt->evt_id){
            case PM_EVT_CONN_SEC_SUCCEEDED :
                ble_bps_is_indication_enabled(&m_bps, &is_indication_enabled);
                if(is_indication_enabled)   {
                    blood_pressure_measurement_send();
                }
                break ;
            case PM_EVT_PEER_DELETE_SUCCEEDED :
                advertising_start(false);
                break ;
            default :
                break ;
        }
    }

static void peer_manager_init()  {
        pm_init();
        ble_gap_sec_params_t sec_params = { 0 };
        sec_params.bond = 1;
        sec_params.io_caps = BLE_GAP_IO_CAPS_NONE ;
        sec_params.kdist_own.enc = 1 ;
        sec_params.kdist_own.id = 1 ;
        sec_params.kdist_peer.enc = 1 ;
        sec_params.kdist_peer.id = 1;
        sec_params.keypress = 0 ;
        sec_params.lesc = 0;
        sec_params.max_key_size = 16 ;
        sec_params.min_key_size = 7 ;
        sec_params.mitm = 0 ;
        sec_params.oob = 0 ;
    pm_sec_params_set(&sec_params);
    pm_register(hndl_evt_pm);
}

    static void hndl_err_conn_params()  {}
    static void hndl_evt_conn_params()  {}
static void conn_params_init() {
        ble_conn_params_init_t cp_init = { 0 };
        cp_init.disconnect_on_fail = false ;
        cp_init.error_handler = hndl_err_conn_params;
        cp_init.evt_handler = hndl_evt_conn_params ;
        cp_init.max_conn_params_update_count = 3;
        cp_init.p_conn_params = NULL ;
        cp_init.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID ;
        cp_init.first_conn_params_update_delay = APP_TIMER_TICKS(5000);
        cp_init.next_conn_params_update_delay = APP_TIMER_TICKS(30000);
    ble_conn_params_init(&cp_init);
}

    ble_uuid_t adv_uuids[] = { 
        { BLE_UUID_BLOOD_PRESSURE_SERVICE, BLE_UUID_TYPE_BLE}, 
        { BLE_UUID_BATTERY_SERVICE, BLE_UUID_TYPE_BLE}, 
        { BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE} 
    };

    static void hndl_evt_advertising()  {   ;   }
static void advertising_init() {
        ble_advertising_init_t init = { 0 };
        init.advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE ;
        init.advdata.include_appearance = false ;
        init.advdata.name_type = BLE_ADVDATA_FULL_NAME ;
        init.config.ble_adv_fast_enabled = true ;
        init.config.ble_adv_fast_interval = 40 ;
        init.config.ble_adv_fast_timeout = 18000 ;
        init.evt_handler = hndl_evt_advertising ;
        init.srdata.uuids_complete.p_uuids = adv_uuids;
        init.srdata.uuids_complete.uuid_cnt = sizeof(adv_uuids)/sizeof(adv_uuids[0]);
    ble_advertising_init( &m_advertising, &init ) ;
    ble_advertising_conn_cfg_tag_set( &m_advertising, 1);
}

    static sensorsim_cfg_t          m_battery_sim_cfg = { 0 };
    static sensorsim_state_t        m_battery_sim_state = { 0 };
static void sensor_simulator_init()    {
        m_battery_sim_cfg.min = 81;
        m_battery_sim_cfg.max = 100;
        m_battery_sim_cfg.incr = 1;
        m_battery_sim_cfg.start_at_max = true ;
    sensorsim_init(&m_battery_sim_state, &m_battery_sim_cfg);

        bps_meas_sim_val[0].systolic.mantissa =         117;
        bps_meas_sim_val[0].diastolic.mantissa =        76;
        bps_meas_sim_val[0].mean_arterial.mantissa =    103;
        bps_meas_sim_val[0].pulse_rate.mantissa =       60;
        //==================================================
        bps_meas_sim_val[1].systolic.mantissa =         121;
        bps_meas_sim_val[1].diastolic.mantissa =        81;
        bps_meas_sim_val[1].mean_arterial.mantissa =    106;
        bps_meas_sim_val[1].pulse_rate.mantissa =       72;
        //==================================================
        bps_meas_sim_val[2].systolic.mantissa =         138;
        bps_meas_sim_val[2].diastolic.mantissa =        88;
        bps_meas_sim_val[2].mean_arterial.mantissa =    120;
        bps_meas_sim_val[2].pulse_rate.mantissa =       105;
        //==================================================
        bps_meas_sim_val[3].systolic.mantissa =         145;
        bps_meas_sim_val[3].diastolic.mantissa =        100;
        bps_meas_sim_val[3].mean_arterial.mantissa =    131;
        bps_meas_sim_val[3].pulse_rate.mantissa =       125;
}

    static void hndl_err_qwr()  {    ;   };
    static void hndl_evt_bps(ble_bps_t *p_bps, ble_bps_evt_t *p_evt)  {
        switch (p_evt->evt_type) {
            case BLE_BPS_EVT_INDICATION_ENABLED :
                blood_pressure_measurement_send();
                break;
            case BLE_BPS_EVT_INDICATION_CONFIRMED :
                m_bps_meas_ind_conf_pending = false ;
                break;
            default :
                break;
        }
    };
    BLE_BPS_DEF(m_bps);
    BLE_BAS_DEF(m_bas);
    NRF_BLE_QWR_DEF(m_qwr);
static void services_init() {
        ble_bps_init_t bps_init = { 0 };
        ble_bas_init_t bas_init = { 0 };
        ble_dis_init_t dis_init = { 0 };
        ble_dis_sys_id_t sys_id = { 0 };
        nrf_ble_qwr_init_t qwr_init = { .error_handler = hndl_err_qwr };

    nrf_ble_qwr_init(&m_qwr, &qwr_init);
        bps_init.feature = BLE_BPS_FEATURE_BODY_MOVEMENT_BIT | BLE_BPS_FEATURE_MEASUREMENT_POSITION_BIT ;
        bps_init.bp_cccd_wr_sec = SEC_JUST_WORKS ; 
        bps_init.bp_feature_rd_sec = SEC_OPEN ; 
        bps_init.evt_handler = hndl_evt_bps ;
    ble_bps_init(&m_bps,&bps_init);
        bas_init.bl_cccd_wr_sec = SEC_OPEN;
        bas_init.bl_rd_sec = SEC_OPEN;
        bas_init.bl_report_rd_sec = SEC_OPEN;
        bas_init.evt_handler = NULL ;
        bas_init.p_report_ref = NULL ;
        bas_init.support_notification = true;
        bas_init.initial_batt_level = 100;
    ble_bas_init(&m_bas, &bas_init);
        ble_srv_ascii_to_utf8( & dis_init.manufact_name_str, "NordicSemiconductor");
        ble_srv_ascii_to_utf8( & dis_init.model_num_str , "NS-BPS-Example") ;
            sys_id.manufacturer_id = 0x1122334455 ;
            sys_id.organizationally_unique_id = 0x7788 ;
        dis_init.p_sys_id = &sys_id ; 
        dis_init.dis_char_rd_sec = SEC_OPEN;
    ble_dis_init(&dis_init);
}

    NRF_BLE_GATT_DEF(m_gatt);
static void gatt_init() {
    nrf_ble_gatt_init(&m_gatt,NULL);
}

static void gap_params_init()   {
        ble_gap_conn_sec_mode_t sec_mode = { 0 } ;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    sd_ble_gap_device_name_set(&sec_mode,"Nordic_BPS",strlen("Nordic_BPS"));
    sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_BLOOD_PRESSURE);
        ble_gap_conn_params_t conn_params = { 0 };  
        {
            conn_params.conn_sup_timeout = MSEC_TO_UNITS(4000,UNIT_10_MS);
            conn_params.max_conn_interval = MSEC_TO_UNITS(1000, UNIT_1_25_MS);
            conn_params.min_conn_interval = MSEC_TO_UNITS(500, UNIT_1_25_MS);
            conn_params.slave_latency = 0;
        }
    sd_ble_gap_ppcp_set(&conn_params);
}

    static void hndl_evt_ble()  {   ;   };
static void ble_stack_init() {
    nrf_sdh_enable_request();
        uint32_t ram_start = 0;
    nrf_sdh_ble_default_cfg_set(1,&ram_start);
    nrf_sdh_ble_enable(&ram_start);
    NRF_SDH_BLE_OBSERVER(m_observer,3,hndl_evt_ble,NULL)
}

static void application_timers_start() {
    app_timer_start(m_timer_battery, APP_TIMER_TICKS(2000), NULL );
}

    APP_TIMER_DEF(m_timer_battery);
    static void hndl_evt_batt() {

    }
static void timers_init() {
    app_timer_init();
    app_timer_create(&m_timer_battery,APP_TIMER_MODE_REPEATED,hndl_evt_batt);
}