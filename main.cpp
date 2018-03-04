
/* mbed Example Program
 * Copyright (c) 2006-2014 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/******************************************************************* Includes */
#include "mbed.h"
#include "rtos.h"
#include "EthernetInterface.h"
#include "NTPClient.h"
#include "TCPServer.h"
#include "TCPSocket.h"

/******************************************************************** Defines */
#define NTP_UPDATE_TIME         0x01
#define STATIC_IP

/************************************************************** HTTP Response */
static char http_response[2048];
static const char http_status_line[] = "HTTP/1.0 200 OK\r\n";
static const char http_header_fields[] = "Content-Type: text/html; charset=utf-8\r\n\r\n";
static char http_msg_body[] = ""                                                    \
        "<html>" "\r\n"                                                             \
        "  <body style=\"display:flex;text-align:center\">" "\r\n"                  \
        "    <div style=\"margin:auto\">" "\r\n"                                    \
        "      <h1>HTTP Server Example</h1>" "\r\n"                                 \
        "      <p>Toggle the Led's on the XMC4500 Relax Kit !</p>" "\r\n"           \
        "      <button id=\"toggle1\">Toggle LED1</button>" "\r\n"                  \
        "      <button id=\"toggle2\">Toggle LED2</button>" "\r\n"                  \
        "      <p>Button 1: Inactive</p>" "\r\n"                                    \
        "      <p>Button 2: Inactive</p>" "\r\n"                                    \
        "      <p>Program Network Time into RTC</p>" "\r\n"                         \
        "      <button id=\"set_time\">Set Time</button>" "\r\n"                    \
        "      <p>Actual System Time: Sat Mar 3 20:25:32 2018  </p>" "\r\n"         \
        "    </div> <br>" "\r\n"                                                    \
        "  </body>" "\r\n"                                                          \
        "  <br>" "\r\n"                                                             \
        "  <script>document.querySelector('#toggle1').onclick = function() {"       \
        "  var x1 = new XMLHttpRequest(); x1.open('POST', '/toggle1'); x1.send();"  \
        "  }</script>"                                                              \
        "  <script>document.querySelector('#toggle2').onclick = function() {"       \
        "  var x2 = new XMLHttpRequest(); x2.open('POST', '/toggle2'); x2.send();"  \
        "  }</script>"                                                              \
        "  <script>document.querySelector('#set_time').onclick = function() {"      \
        "  var x3 = new XMLHttpRequest(); x3.open('POST', '/set_time'); x3.send();" \
        "  }</script>"                                                              \
        "  <script>var myVar = setInterval(myTimer, 1000);"                         \
        "  function myTimer() {"                                                    \
        "  location.reload(true); "                                                 \
        "  }</script>"                                                              \
        "</html>";

/******************************************************************** Globals */
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalIn sw1(SW1);
DigitalIn sw2(SW2);
Thread ntp_thread;
Thread bt_thread;
EventFlags ntp_update_event;

/****************************************************************** Functions */

/**
 * Change timestamp in http source
 */
static void change_button_state_in_http_source(void){

    char *ret;

    /* Change button 1 state */
    ret = strstr(http_msg_body, "Button 1:");
    if(!sw1){
        strncpy(ret + 10, "Active  ", 8);
    }else{
        strncpy(ret + 10, "Inactive", 8);
    }

    /* Change button 2 state */
    ret = strstr(http_msg_body, "Button 2:");
    if(!sw2){
        strncpy(ret + 10, "Active  ", 8);
    }else{
        strncpy(ret + 10, "Inactive", 8);
    }

    /* Build new http response */
    sprintf(http_response, "%s%s%s",http_status_line, http_header_fields, http_msg_body);
}

/**
 * Change buttons state in http source
 */
static void change_timestamp_in_http_source(void){

    char *ret;

    /* Get current time from RTC */
    time_t seconds = time(NULL);

    /* Change time string in http_msg_body[] */
    ret = strstr(http_msg_body, "Time:");
    strncpy(ret + 6, ctime(&seconds), 25);

    /* Build new http response */
    sprintf(http_response, "%s%s%s",http_status_line, http_header_fields, http_msg_body);
}

/**
 * NTP Client Thread
 */
void ntp_client_thread(EthernetInterface *eth) {

    NTPClient ntp(eth);
    time_t timestamp;

    while (true) {
        /* Wait until request from main thread */
        ntp_update_event.wait_all(NTP_UPDATE_TIME);
        /* Get Time from NTP server */
        timestamp = ntp.get_timestamp(5000);
        if (timestamp < 0) {
            printf("An error occurred when getting the time. Code: %ld\r\n", timestamp);
        } else {
            /* Set RTC Time */
            set_time(timestamp);
            /* Print Time */
            printf("Current time is %s\r\n", ctime(&timestamp));
        }
    }
}

/**
 * Update Button state thread
 */
void button_thread(void) {

    while (true) {

        wait(1);
        change_button_state_in_http_source();
        change_timestamp_in_http_source();
    }
}

/**
 * Main Function
 */
int main()
{
    TCPServer srv;
    TCPSocket clt_sock;
    SocketAddress clt_addr;
    EthernetInterface eth;
    char rec_buffer[256];
    int rcount;

    printf("Basic HTTP server example\n");

#ifdef STATIC_IP
    eth.set_network("192.168.1.40", "255.255.255.0", "192.168.1.2");
#endif

    /* Initialize low level Ethernet driver and lwip stack */
    eth.connect();

    /* Print target IP address */
    printf("The target IP address is '%s'\n", eth.get_ip_address());

    /* Open the server on ethernet stack */
    srv.open(&eth);

    /* Bind the HTTP port (TCP 80) to the server */
    srv.bind(eth.get_ip_address(), 80);

    /* Can handle 5 simultaneous connections */
    srv.listen(5);

    /* Set RTC time to 1 Jan 1970 */
    set_time(0);
    /* Update timestamp in http source */
    change_timestamp_in_http_source();

    /* start NTP client thread */
    ntp_thread.start(callback(ntp_client_thread, &eth));

    /* start button update thread */
    bt_thread.start(button_thread);

    while (true) {
        /* Wait until request from client */
        srv.accept(&clt_sock, &clt_addr);
        printf("accept %s:%d\n", clt_addr.get_ip_address(), clt_addr.get_port());

        /* Get receive message */
        rcount = clt_sock.recv(rec_buffer, sizeof rec_buffer);
        printf("recv %d [%.*s]\r\n", rcount, strstr(rec_buffer, "\r\n")-rec_buffer, rec_buffer);
        /* Check if LED1 toggle button was clicked */
        if(strstr(rec_buffer, "toggle1") != NULL){
            led1 = !led1;
            continue;
        }
        /* Check if LED2 toggle button was clicked */
        if(strstr(rec_buffer, "toggle2") != NULL){
            led2 = !led2;
            continue;
        }
        /* Check if Set Time button was clicked */
        if(strstr(rec_buffer, "set_time") != NULL){
            /* Notify NTP client thread to update time */
            ntp_update_event.set(NTP_UPDATE_TIME);
            continue;
        }
        /* Send HTTP response */
        clt_sock.send(http_response, strlen(http_response));
    }
}

/*EOF*/
