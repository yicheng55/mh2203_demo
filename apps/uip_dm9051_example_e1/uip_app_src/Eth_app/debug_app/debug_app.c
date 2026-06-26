//.\BoardApplications\DM9051_uip_app\Eth_app\debug_app

#include  <stdio.h>
#include "uip.h"

void debug_appcall(char *dbgHead)
{
 //struct httpd_state *s = (struct httpd_state *)&(uip_conn->appstate);
 //int i;

	 //[more for printing.]
	 if (uip_aborted()) {
		 printf("%s, ", dbgHead); //[headstr]
		 printf("[TCP.Client wanta abort 5002]b. uip_flags %02x\r\n", uip_flags); //[more for conns list.]//_tcp_conn_list("uip-aborted found", 3);
	 }
	
	 if(uip_closed() || uip_aborted() || uip_timedout()) {
		 printf("%s, ", dbgHead); //[headstr]
		 if (uip_poll()){
			 printf(";_dbg;found;uip-poll;");
		 } else {
			 printf(";_dbg;found;directly;");
		 }
		 if (uip_timedout()) printf(";uip-timedout");
		 if (uip_aborted()) printf(";uip-aborted");
		 if (uip_closed()) printf(";uip-closed");
		 printf("\r\n");
	 }
 
 if(uip_connected()) {
	//printf("%s, ", dbgHead); //[headstr]

	//uip_send("Hello",5);  
	//printf("NOT as uip_send(\"Hello\",5)\r\n");
	 
	//if (uip_newdata())
	//	printf("Unused before connected uip_len = %d\r\n",uip_len);
	return;
 }
 
 if(uip_newdata()) {
	//printf("%s, ", dbgHead); //[headstr]

	//printf("%s\r\n", uip_appdata);
    //uip_send(uip_appdata,uip_len);
	 
	//uip_send("Hello",5);  
		
  
 } /*else if(s != NULL) {
	//printf("%s, ", dbgHead); //[headstr]
    if(uip_poll()) {
      ++s->timer;
      if(s->timer >= 20) {
         uip_abort();	
      }
    } else {
      s->timer = 0;
    }
   } else {
	//printf("%s, ", dbgHead); //[headstr]
    uip_abort();
  }*/
  
}

void debug_appcall_after(char *dbgHead)
{
	char compose[50];
	if(uip_connected()){
		sprintf(compose, "%s%s", dbgHead, ",since Connected");
		tcp_conn_list(compose, 3); //e.g. compose lstrcat "Connected"
	}
 if(uip_closed() || uip_aborted() || uip_timedout()) {
	 //if (uip_poll()){
		// printf(";uip-poll;");
	 //} else {
		// printf(";directly;");
	 //}
	 //if (uip_timedout()) printf(";uip-timedout");
	 //if (uip_aborted()) printf(";uip-aborted");
	 //if (uip_closed()) printf(";uip-closed");
	 //printf("\r\n");

	 //[more for printing.]
	 if (uip_aborted())
		 printf("[TCP.Client wanta abort 5002]a. uip_flags %02x\r\n", uip_flags); //[more for conns list.] //_tcp_conn_list("uip-aborted found", 3);
 }
}
