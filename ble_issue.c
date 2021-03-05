#include <eason-s140.h>

#define APP_BLE_CONN_CFG_TAG    1
#define APP_BLE_OBSERVER_PRIO   3

#define DEVICE_NAME             "Nordic Blinky"

#define MAX_CONN_INTERVAL MSEC_TO_UNITS(100, UNITS_1_25_MS)     //0.5s
#define MIN_CONN_INTERVAL MSEC_TO_UNITS(200,UNITS_1_25_MS)      //1s
#define CONN_SUP_TIMEOUT MSEC_TO_UNITS(4000,UNITS_10_MS)        //4s

#define SLAVE_LATENCY           0

#define APP_ADV_INTERVAL        64
#define APP_ADV_DURATION        BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(20000)
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000)
#define MAX_CONN_PARAMS_UPDATE_COUNT    3

BLE_LBS_DEF(m_lbs);
NRF_BLE_GATT_DEF(m_gatt);
NRF_BLE_QWR_DEF(m_qwr);

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID ; 

static uint8_t m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET ;
static uint8_t m_enc_advdata[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
static uint8_t m_enc_scan_resp_data[BLE_GAP_ADV_SET_DATA_SIZE_MAX];

static ble_gap_adv_data_t m_adv_data = {
    .adv_data={
        .p_data = m_enc_advdata,
        .len = BLE_GAP_ADV_SET_DATA_SIZE_MAX
    },
    .scan_rsp_data={
        .p_data = m_enc_scan_resp_data,
        .len = BLE_GAP_ADV_SET_DATA_SIZE_MAX
    }
};

static void advertising_start()                                         //adv start
{
    sd_ble_gap_adv_start(m_adv_handle,APP_BLE_CONN_CFG_TAG);
}

static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    ret_code_t err_code;
    if(p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED){
        err_code = sd_ble_gap_disconnect(m_conn_handle,BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}

static void conn_params_error_handler(uint32_t nrf_err)     {       APP_ERROR_HANDLER(nrf_error);   }

static void conn_params_init()                                          //conn params init
{
    ble_conn_params_init_t cp_init={0};
    cp_init.disconnect_on_fail = false;
    cp_init.error_handler = ;
    cp_init.evt_handler = conn_params_error_handler;
    cp_init.max_conn_params_update_count = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.p_conn_params = NULL;
    cp_init.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID;
    cp_init.first_conn_params_update = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update = NEXT_CONN_PARAMS_UPDATE_DELAY;

    ble_conn_params_init();
}

static void advertising_init()                                          //adv set configure
{
    ret_code_t err_code ;
    ble_advdata_t advdata = {0} , srdata = {0};
        advdata.name_type = BLE_ADVDATA_FULL_NAME;
        advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
        advdata.include_appearance = true;

    ble_uuid_t adv_uuids[] = {{LBS_UUID_SERVICE,m_lbs.uuid_type}};
        srdata.uuids_complete.p_uuids = adv_uuids;
        srdata.uuids_complete.uuid_cnt = sizeof(adv_uuids)/sizeof(adv_uuids[0]);

    err_code = ble_advdata_encode(&advdata,m_adv_data.adv_data.p_data, m_adv_data.adv_data.len);
    APP_ERROR_CHECK(err_code);
    err_code = ble_advdata_encode(&srdata, m_adv_data.scan_rsp_data.p_data, m_adv_data.scan_rsp_data.len);
    APP_ERROR_CHECK(err_code);

    ble_gap_adv_params_t adv_params={0};
        adv_params.primary_phy = BLE_GAP_PHY_1MBPS;
        adv_params.duration = APP_ADV_DURATION;
        adv_params.properties.type = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
        adv_params.p_peer_addr = NULL ;
        adv_params.filter_policy = BLE_GAP_ADV_FP_ANY;
        adv_params.interval = APP_ADV_INTERVAL;
    err_code = sd_ble_gap_adv_set_configure(&m_adv_handle,&m_adv_data,);
    APP_ERROR_CHECK(err_code);
}

static void nrf_qwr_error_handler(uint32_t nrf_error)       {       APP_ERROR_HANDLER(nrf_error);       }

static void lbs_write_handler(uint16_t conn_handle, ble_lbs_t *p_lbs, uint8_t led_state)
{
    if(led_state){
        NRF_LOG_INFO("Received LED ON");
    }else{
        NRF_LOG_INFO("Received LED OFF");
    }
}

static void services_init()                                     //qwr and specific service init
{
    ret_code_t err_code;
    nrf_ble_qwr_init_t qwr_init ={0};
    ble_lbs_init_t  lbs_init={0};
    qwr_init.error_handler = nrf_qwr_error_handler;
    err_code = nrf_ble_qwr_init(&m_qwr,&qwr_init);
    APP_ERROR_CHECK(err_code);

    // nrf_ble_gatt_
    // sd_ble_gattc/s
    lbs_init.led_write_handler = lbs_write_handler;
    err_code = ble_lbs_init(&m_lbs,&lbs_init);
    APP_ERROR_CHECK(err_code);
}

// ble issue
static void gatt_init()                                         //gatt defined
{
    ret_code_t err_code;
    err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}

static void gap_params_init()                                   //gap device defined by sec_mode and ppcp set
{
    ret_code_t err_code ; 
    ble_gap_conn_params_t gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    err_code = sd_ble_gap_device_name_set(&sec_mode,(const uint8_t*)DEVICE_NAME,sizeof(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    mem_set(&gap_conn_params,0,sizeof(gap_conn_params));
    gap_conn_params.conn_sup_timeout = CONN_SUP_TIMEOUT;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.slave_latency  = SLAVE_LATENCY;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

static void ble_evt_handler(ble_evt_t *p_ble_evt, void * p_context)
{
    ret_code_t err_code ;
    switch(p_ble_evt->header.evt_id){
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("BLE_GAP_EVT_CONNECTED :: Connected");
            NRF_LOG_INFO("Simulated LED On");
            NRF_LOG_INFO("Simulated LED Off");
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle ;

            ACEE(nrf_ble_qwr_conn_handle_assign(&m_qwr,m_conn_handle));
            ACEE(app_button_enable());                                      //psuedo button
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("BLE_GAP_EVT_DISCONNECTED :: Disconnected");
            NRF_LOG_INFO("Simulated LED Off");
            m_conn_handle = BLE_CONN_HANDLE_INVALID;

            ACEE(app_button_disable());
            advertising_start();                                            //why start advertising ?
            break;
        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            NRF_LOG_INFO("BLE_GAP_EVT_SEC_PARAMS_REQUEST");
            ACEE(sd_ble_gap_sec_params_reply(m_conn_handle,BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP,NULL,NULL));
            break;
        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
            NRF_LOG_INFO("BLE_GAP_EVT_PHY_UPDATE_REQUEST");
            {
                ble_gap_phys_t phy={
                    .rx_phys = BLE_GAP_PHY_AUTO,
                    .tx_phys = BLE_GAP_PHY_AUTO
                };
                ACEE(sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle,&phy));
            }
            break;
        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            NRF_LOG_INFO("BLE_GATTS_EVT_SYS_ATTR_MISSING");
            ACEE(sd_ble_gatts_sys_attr_set(m_conn_handle,NULL,0,0));
            break;
        case BLE_GATTS_EVT_TIMEOUT:
            NRF_LOG_INFO("BLE_GATTS_EVT_TIMEOUT");
            ACEE(sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION));
            break;
        case BLE_GATTC_EVT_TIMEOUT:
            NRF_LOG_LOG("BLE_GATTC_EVT_TIMEOUT");
            ACEE(sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION));
            break;
        default :
            break;
    }
}

static void ble_stack_init()                                    //stack :: dft_cfg , enable, OBSERVER define
{
    ret_code_t err_code ;
    err_code = nrf_sdh_enable_request
    APP_ERROR_CHECK(error_code);

    uint32_t ram_start ; 
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    err_code=nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    NRF_SDH_BLE_OBSERVER(m_ble_observer,APP_BLE_OBSERVER_PRIO,ble_evt_handler,NULL);

}