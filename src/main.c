#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>

#define ADC_NODE 	DT_NODELABEL(adc)
static const struct device *adc_dev = DEVICE_DT_GET(ADC_NODE);

#define ADC_RESOLUTION 10							//0-1023
#define ADC_CHANNEL		0
#define ADC_PORT SAADC_CH_PSELP_PSELP_AnalogInput0 //AIN0
#define ADC_REFERENCE ADC_REF_INTERNAL				//0.6V
#define ADC_GAIN ADC_GAIN_1_5						//ADC_REFERENCE*5

struct adc_channel_cfg chnl0_cfg = {
	.gain = ADC_GAIN,
	.reference = ADC_REFERENCE,
	.acquisition_time = ADC_ACQ_TIME_DEFAULT,
	.channel_id = ADC_CHANNEL,
#ifdef CONFIG_ADC_NRFX_SAADC
	.input_positive = ADC_PORT
#endif
};


int16_t sample_buffer[1];
struct adc_sequence sequence = {
	.channels = BIT(ADC_CHANNEL),
	.buffer = sample_buffer,
	.buffer_size = sizeof(sample_buffer),
	.resolution = ADC_RESOLUTION
};


void main(void)
{
	int err;

	if (!device_is_ready(adc_dev)) {
		printk("ADC controller device not ready\n");
		return;
	}

	err = adc_channel_setup(adc_dev, &chnl0_cfg);
	if (err < 0) {
		printk("Could not setup channel #%d\n", err);
		return;
	}
	

	while (1) {
		printk("ADC reading:\n");
		err = adc_read(adc_dev, &sequence);
		if (err != 0) {
			printk("Could not read (%d)\n", err);
			continue;
		} 
		else {
			printk("sample buffer = %d\n", sample_buffer);
		}



		/* conversion to mV may not be supported, skip if not */
		int32_t val_mv = sample_buffer[0];

		printk("ADC value = %d\n", val_mv);
		
		int32_t adc_vref = adc_ref_internal(adc_dev);

		err = adc_raw_to_millivolts(adc_vref, ADC_GAIN, ADC_RESOLUTION, &val_mv);
		if (err < 0) {
				printk(" (value in mV not available)\n");
			} else {
				printk("ADC voltage = %d mV\n", val_mv);
			}
		}

		k_sleep(K_MSEC(10000));
}
