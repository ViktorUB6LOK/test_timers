/*
 * ad9833_app.h
 *
 *  Created on: Dec 3, 2025
 *      Author: user
 */

#ifndef AD9833_APP_H_
#define AD9833_APP_H_

#include "main.h"
#include "ad9833.h"
#include "setting.h"

void send_data_to_ad9833   (uint16_t data_flowmeter, uint16_t setting_ratio_flowmeter);
void send_data_to_ad9833_2 (uint16_t data_speedmeter, uint16_t setting_ratio_speedmeter);
void power_off_ad9833 (void);
void power_off_ad9833_2 (void);

#endif /* AD9833_APP_H_ */
