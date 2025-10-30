/**
 * @file network_time.c
 * @author shameem
 * @brief
 * @date 2025-06-26
 * @copyright Copyright (c) 2025
 */

/************************************************************* Header includes ***************************************************************/
#include "network_time.h"
/***************************************************************** Macros ********************************************************************/


#define NTP_PORT            (123U)
#define IST_5_30            (19800U)
#define NTP_DELTA           (2208988800UL) // Difference between 1900 and 1970
#define NTP_SERVER          ("time.google.com")
#define NTP_TIMEOUT         (2000U)
#define NETWORK_TIME_ERROR  (-1)
/******************************************************************* Variables ******************************************************************/
// Days before each month (non-leap year)
static const int ai32_days_before_month[12] =
{
        0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
};


/************************************************************ static Function prototypes *******************************************************/
static int is_leap(int i32_year);

/************************************************************ Function Definitions *************************************************************/


/*************************************************************************************************************************************************
 * @brief This function determines the given year is leap year or not 
 * @param i32_year - year to be check wether its leap year
 * @return int     - HIGH if its leap year 
 *                 - LOW if its not leap year
 *************************************************************************************************************************************************/
static int is_leap(int i32_year)
{
    return (i32_year % 4 == 0 && (i32_year % 100 != 0 || i32_year % 400 == 0));
}


/*************************************************************************************************************************************************
 * @brief This function will process u32_timestamp and find all the time related parameter from it.
 * 
 * @param u32_timestamp - This variable will having calculated seconds from 1970 jan 1
 * @param out_tm        - The processed time related parameters will be stored in this structure
 *************************************************************************************************************************************************/
void unix_time_to_tm(uint32_t u32_timestamp, struct tm *out_tm)
{
    int i32_year                = RESET;
    int i32_month               = RESET;
    int i32_days_in_a_year      = RESET;
    int i32_days_in_a_month     = RESET;

    uint8_t u8_year_found_flag  = RESET;
    uint8_t u8_month_found_flag = RESET;

    uint32_t u32_days_from_1970 = RESET;
    uint32_t u32_remaining_sec  = RESET;

    // 1. Break total seconds into u32_days_from_1970 and remaining seconds of the current day
    u32_days_from_1970 = u32_timestamp / 86400; // Total number of days since Jan 1, 1970
    u32_remaining_sec  = u32_timestamp % 86400;  // Remaining seconds within the current day

    // 2. Convert remaining seconds into HH:MM:SS
    out_tm->tm_hour = u32_remaining_sec / 3600; // Hours (0-23)
    u32_remaining_sec %= 3600;
    out_tm->tm_min = u32_remaining_sec / 60; // Minutes (0-59)
    out_tm->tm_sec = u32_remaining_sec % 60; // Seconds (0-59)

    // 3. Calculate current year by subtracting full years worth of days
    i32_year = 1970; // Start from UNIX epoch
    while (u8_year_found_flag != SET)
    {
        i32_days_in_a_year = is_leap(i32_year) ? 366 : 365; // Get days in this year (leap or not)
        if (u32_days_from_1970 < i32_days_in_a_year)
        {
            u8_year_found_flag = SET; // Stop if current year is found
        }
        else
        {
            u32_days_from_1970 -= i32_days_in_a_year; // Remove full i32_year's days
            i32_year++;        // Move to next year
        }
    }

    out_tm->tm_year = i32_year - 1900; // Store years since 1900 (as per `struct tm`)
    out_tm->tm_yday = u32_days_from_1970;        // Day of the year (0–365)

    // 4. Determine month and day of the month
    while(u8_month_found_flag != SET)
    {
        i32_days_in_a_month = ai32_days_before_month[i32_month + 1] - ai32_days_before_month[i32_month];
        if (i32_month == 1 && is_leap(i32_year))
        {
            i32_days_in_a_month++; // Add one day if Feb and leap year
        }

        if (u32_days_from_1970 < i32_days_in_a_month)
        {
            u8_month_found_flag = SET;             // Found correct month
        }
        else
        {
            u32_days_from_1970 -= i32_days_in_a_month; // Subtract full month's days
        }

        if(i32_month >= 11)
        {
            u8_month_found_flag = SET;  
        }
        else
        {
           i32_month++;
        }
    }

    out_tm->tm_mon = i32_month;     // Month number (0–11)
    out_tm->tm_mday = u32_days_from_1970 + 1; // Day of the month (1–31)

    // 5. Compute day of week (0 = Sunday)
    // Formula derived from known weekday of UNIX epoch (Thursday = 4)
    out_tm->tm_wday = (4 +                   // Jan 1, 1970 was Thursday
                       out_tm->tm_yday +     // Days passed in current i32_year
                       365 * (i32_year - 1970) + // Full non-leap years since 1970
                       (i32_year - 1969) / 4 -   // Add 1 day for each leap year since 1970
                       (i32_year - 1901) / 100 + // Subtract 1 day for each skipped leap year
                       (i32_year - 1601) / 400   // Add 1 day back for leap centuries
                       ) %
                      7; // Modulo 7 gives day of the week

    out_tm->tm_isdst = 0; // Daylight saving time not supported (set to 0)
}


/*************************************************************************************************************************************************
 * @brief This function will handle communication with ntp server and convert the received data to unix
 * 
 * @return int 
 *************************************************************************************************************************************************/
int fetch_network_time(void)
{   
    int i32_ret = RESET;
   
    // for udp
    int i32_socket                 = CLEAR;
    int i32_packet_len             = CLEAR;
    ip_addr_t ip_address           = {0};
    struct sockaddr_in server_addr = {0};

    //NTP
    time_t t_ist_time               = CLEAR;
    time_t t_unix_time              = CLEAR;
    uint32_t u32_seconds_since_1900 = CLEAR;
    struct tm timeinfo              = {0};
    uint8_t au8_ntp_packet[48]      = {0x1B, 0}; // LI = 0, VN = 3, Mode = 3 (client)
    uint8_t au8_printf_buff[110]        = {0}; 

    //creating sockeyt for udp communication with ntp server with port 123
    i32_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (i32_socket < 0)
    {
        i32_ret = NETWORK_TIME_ERROR;
    }
    else
    {

        // converting server url to ip
        if ((i32_ret = netconn_gethostbyname(NTP_SERVER, &ip_address)) >= 0)
        {

            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(NTP_PORT); // 123 for ntp prototcol
            server_addr.sin_addr.s_addr = ip_address.addr;

            // request for utc time NTP_SERVER with cmd 0x1B
            if (sendto(i32_socket, au8_ntp_packet, sizeof(au8_ntp_packet), 0,
                       (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
            {
                i32_ret = NETWORK_TIME_ERROR;
            }
            else
            {

                //clearing buff
                buff_clr(au8_ntp_packet,sizeof(au8_ntp_packet));

                // seting timeout for receving
                struct timeval timeout = {.tv_sec = 2, .tv_usec = 0};
                setsockopt(i32_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

                //reading current time resposne from ntp server
                i32_packet_len = recv(i32_socket, au8_ntp_packet, sizeof(au8_ntp_packet), 0);
                if (i32_packet_len >= 48)
                {
                    //segregating date time portion from ntp packet
                    u32_seconds_since_1900 =
                        (au8_ntp_packet[40] << 24) | (au8_ntp_packet[41] << 16) | (au8_ntp_packet[42] << 8) | au8_ntp_packet[43];

                    //convert teh ntp packet data to unix calculate second from 1970 from second from 1900    
                    t_unix_time = u32_seconds_since_1900 - NTP_DELTA; // unix time convertion (from 1970)

                    t_ist_time = t_unix_time + IST_5_30;

                    unix_time_to_tm(t_ist_time, &timeinfo);

                    sprintf(au8_printf_buff, "\r\n IST Time: %04d-%02d-%02d %02d:%02d:%02d\n",
                            timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                            timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
                    debug_msg(au8_printf_buff);
                }
                else
                {
                    i32_ret = NETWORK_TIME_ERROR;
                }
            }
        }
        closesocket(i32_socket);
    }
    return i32_ret;
}

