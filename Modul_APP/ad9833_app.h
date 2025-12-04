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

void send_data_to_ad9833   (uint16_t data_flowmeter, uint16_t setting_ratio_flowmeter);
void send_data_to_ad9833_2 (uint16_t data_flowmeter, uint16_t setting_ratio_flowmeter);
#endif /* AD9833_APP_H_ */
