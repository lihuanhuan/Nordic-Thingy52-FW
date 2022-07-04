#include "data_transmission.h"
#include "nrf_drv_spi.h"
#include "nrf_drv_gpiote.h"
#include "app_error.h"
#include "nrf_delay.h"

static volatile bool spi_xfer_done = true;  										
static const nrfx_spim_t m_spim_master = NRFX_SPIM_INSTANCE(SPI_INSTANCE);
static nrfx_spim_xfer_desc_t     driver_spim_xfer;
static uint8_t                  driver_spi_rx_buf[256];
static uint8_t 					driver_spi_tx_buf[256];

void usr_spim_init(void)
{
    ret_code_t err_code;

    nrfx_spim_config_t  driver_spi_config = NRFX_SPIM_DEFAULT_CONFIG;
    driver_spi_config.ss_pin   = TOUCH_SPI2_CSN_IO;
    driver_spi_config.miso_pin = TOUCH_SPI2_MISO_IO;
    driver_spi_config.mosi_pin = TOUCH_SPI2_MOSI_IO;
    driver_spi_config.sck_pin = TOUCH_SPI2_CLK_IO;
    driver_spi_config.frequency = NRF_SPIM_FREQ_4M;
    err_code = nrfx_spim_init(&m_spim_master, &driver_spi_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);
}

void usr_spi_write(uint8_t *p_buffer, uint32_t size)  
{  

	nrf_drv_gpiote_in_event_disable(SLAVE_SPI_RSP_IO);

	while(0 == nrf_gpio_pin_read(SLAVE_SPI_RSP_IO)){
        nrf_delay_us(5);
	}

	while(true){
		if(size<=255){
			driver_spim_xfer.tx_length   = size;  
			driver_spim_xfer.p_tx_buffer = p_buffer;
			driver_spim_xfer.rx_length   = 0;
			driver_spim_xfer.p_rx_buffer = driver_spi_rx_buf;  
			APP_ERROR_CHECK(nrfx_spim_xfer(&m_spim_master, &driver_spim_xfer, 0));
			break;
		}else{
			driver_spim_xfer.tx_length   = 255;  
			driver_spim_xfer.p_tx_buffer = p_buffer;
			driver_spim_xfer.rx_length   = 0;
			driver_spim_xfer.p_rx_buffer = driver_spi_rx_buf;  
			APP_ERROR_CHECK(nrfx_spim_xfer(&m_spim_master, &driver_spim_xfer, 0));
			p_buffer += 255;
			size -= 255;
		}
	}

	while(1 == nrf_gpio_pin_read(SLAVE_SPI_RSP_IO)){
		nrf_delay_us(5);
	}
	nrf_drv_gpiote_in_event_enable(SLAVE_SPI_RSP_IO, true);

}
void usr_spi_read(uint8_t *p_buffer, uint32_t size)
{
	driver_spim_xfer.tx_length   = 0;  
	driver_spim_xfer.p_tx_buffer = driver_spi_tx_buf;
	driver_spim_xfer.rx_length   = size;
	driver_spim_xfer.p_rx_buffer = p_buffer;
	APP_ERROR_CHECK(nrfx_spim_xfer(&m_spim_master, &driver_spim_xfer, 0));
}

//Disable spi mode to enter low power mode
void usr_spi_disable(void)
{
    nrfx_spim_uninit(&m_spim_master);
}

