#include "epoch_conv.h"

uint64_t date_to_epoch(char* line){
		// expected format "2018-07-22 17:24:11.316703005"
	uint64_t  ret_res = 0;
	// constants
    const unsigned char month_epoch = 1, day_epoch = 1, hour_poch = 0, minute_poch = 0;
    const int year_epoch = 1970;
    const double sec_ms_epoch = 0;
    unsigned char leap_years =0;
    const unsigned long month_s[] = {2678400, 5097600, 7776000,10368000,13046400,15638400,18316800,20995200,23587200,26265600,28857600,31536000}; // seconds enumeration
    
    char* year = (char*)calloc(5, sizeof(char)); // format 2017 +1 for 0x00
    char* month = (char*)calloc(3, sizeof(char)); // format 12 +1 for 0x00
    char* day = (char*)calloc(3, sizeof(char)); // format 27 +1 for 0x00
    char* hour= (char*)calloc(3, sizeof(char));
    char* minute= (char*)calloc(3, sizeof(char));
    char* sec_ms= (char*)calloc(13, sizeof(char));
    
	memcpy(year, line, 4);
	while(*line++ != '-');
    memcpy(month, line, 2);
    while(*line++ != '-');
    memcpy(day, line, 2);
    while(*line++ != ' ');
    memcpy(hour, line, 2);
    while(*line++ != ':');
    memcpy(minute, line, 2);
    while(*line++ != ':');
    memcpy(sec_ms, line, 12);
    
    // conversion
    unsigned int year_i = atoi(year)             -year_epoch;    // this may differ, compensate for 0th year
    unsigned int month_i = atoi(month)   -month_epoch;   // this may differ
    unsigned int day_i = atoi(day)               -day_epoch;             // this may differ
    unsigned int hour_i = atoi(hour);                            // time offset is always 0, no need to subtract
    unsigned int minute_i = atoi(minute);
    unsigned int sec_ms_i = atof(sec_ms)*1000000.0;      // ns precision dd.mmmnnn
    ret_res += (365*year_i)*86400;
    ret_res += month_s[month_i-1];//2628000;
    ret_res += (day_i)*86400;
    ret_res += hour_i*3600;
    ret_res += minute_i*60;

    if (leap_years == 0) leap_years= ++year_i/4;
    ret_res += (leap_years)*86400;
    ret_res *= 1000000;
    ret_res += (int)sec_ms_i;
    //capture year until - is found
    //capture month until - is found
    //capture day until ' ' is found

    free(year);
    free(month);
    free(day);
    free(hour);
    free(minute);
    free(sec_ms);

    return ret_res;
}

uint64_t convert_date_to_epoch(char* line){ // requires trimmed version
        uint64_t  ret_res = 0;
	// constants
        const unsigned char month_epoch = 1, day_epoch = 1, hour_poch = 0, minute_poch = 0;
        const int year_epoch = 1970;
        const double sec_ms_epoch = 0;
        unsigned char leap_years =0;
        const unsigned long month_s[] = {2678400, 5097600, 7776000,10368000,13046400,15638400,18316800,20995200,23587200,26265600,28857600,31536000}; // seconds enumeration
	//
        char* year = (char*)calloc(5, sizeof(char)); // format 2017 +1 for 0x00
        char* month = (char*)calloc(3, sizeof(char)); // format 12 +1 for 0x00
        char* day = (char*)calloc(3, sizeof(char)); // format 27 +1 for 0x00
        char* hour= (char*)calloc(3, sizeof(char));
        char* minute= (char*)calloc(3, sizeof(char));
        char* sec_ms= (char*)calloc(13, sizeof(char));

        unsigned char month_i, day_i, hour_i, minute_i;
        int year_i = 0;
        double sec_ms_i = 0;
        // 1509006715
        // find first space
        while(*line != ' ') line++; // ignore the padding spaces if they exist and the number
        while(*line == ' ') line++;
        memcpy(year, line, 4); // format 2017
        while(*line++ != '-');
        memcpy(month, line, 2);
        while(*line++ != '-');
        memcpy(day, line, 2);
        while(*line++ != ' ');
        memcpy(hour, line, 2);
        while(*line++ != ':');
        memcpy(minute, line, 2);
        while(*line++ != ':');
        memcpy(sec_ms, line, 12);

        // conversion
        year_i = atoi(year)             -year_epoch;    // this may differ, compensate for 0th year
        month_i = atoi(month)   -month_epoch;   // this may differ
        day_i = atoi(day)               -day_epoch;             // this may differ
        hour_i = atoi(hour);                            // time offset is always 0, no need to subtract
        minute_i = atoi(minute);
        sec_ms_i = atof(sec_ms)*1000000.0;      // ns precision dd.mmmnnn
        ret_res += (365*year_i)*86400;
        ret_res += month_s[month_i-1];//2628000;
        ret_res += (day_i)*86400;
        ret_res += hour_i*3600;
        ret_res += minute_i*60;

        if (leap_years == 0) leap_years= ++year_i/4;
        ret_res += (leap_years)*86400;
        ret_res *= 1000000;
        ret_res += (int)sec_ms_i;
        //capture year until - is found
        //capture month until - is found
        //capture day until ' ' is found

        free(year);
        free(month);
        free(day);
        free(hour);
        free(minute);
        free(sec_ms);

        return ret_res;
}

