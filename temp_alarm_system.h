#ifndef TEMP_ALARM_SYSTEM_H_
#define TEMP_ALARM_SYSTEM_H_

#include "avr/io.h"
#include "utils/data_types.h"

#define TAS_LINE_EDIT		0			//line number(y) in lcd that all editing happens in
#define TAS_TEMP_EDIT		2			//character number (x) that temperature(c) editing happens
#define TAS_THRE_EDIT		8			//character number (x) that threshold(t) editing happens
#define TAS_AACT_EDIT		15			//character number (x) that alaram activation(aa) editing happens

//display functions
void tas_main_disp();
void tas_alar_disp();
void tas_kcon_disp();
void tas_tcon_disp();

//main functions
void tas_init();
void tas_update_temp();
void tas_toggle_aa();
void tas_threshold_edit_key();
void tas_threshold_edit_term();
void tas_run();

#endif /* TEMP_ALARM_SYSTEM_H_ */