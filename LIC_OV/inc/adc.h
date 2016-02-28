void adc_init (void); // initialize AD module
void adc_process(void); // periodicaly measure all channels
word adc_read (byte channel); // measure one channel
word adc_get(byte channel); // get value from periodical scan

// all variables are internal

//extern word adc_data;
//extern word adc_meas[8];

// input definition
#define ADC_p12V (6)
#define ADC_p24V (5)
#define ADC_I0   (1)
#define ADC_I1   (2)
#define ADC_U0   (4)
#define ADC_U1   (3)
