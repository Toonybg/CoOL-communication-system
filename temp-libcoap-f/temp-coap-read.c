int
   coap_read( coap_context_t *ctx ) {
     ssize_t bytes_read = -1;
     coap_packet_t *packet;
     coap_address_t src;
     int result = -1;      /* the value to be returned */
   
     coap_address_init(&src);
  
   #if defined(WITH_POSIX) || defined(WITH_CONTIKI)
     bytes_read = ctx->network_read(ctx->endpoint, &packet);
     /*??? what is network_read doing ???*/
     /**/
   #endif /* WITH_POSIX or WITH_CONTIKI */
   
     if ( bytes_read < 0 ) {
       warn("coap_read: recvfrom");
     } else {
   #if defined(WITH_POSIX) || defined(WITH_CONTIKI)
      /*!!! USE DEBUGGER TO VERIFY THE CONTENTS OF CTX AND PACKET !!!*/
       result = coap_handle_message(ctx, packet);
   #endif /* WITH_POSIX or WITH_CONTIKI */
     }
   
     coap_free_packet(packet);
   
     return result;
   }
  
//****************coap_context_t**********************************
  typedef struct coap_context_t {
      coap_opt_filter_t known_options;
      struct coap_resource_t *resources; 
   #ifndef WITHOUT_ASYNC
   
     struct coap_async_state_t *async_state;
   #endif /* WITHOUT_ASYNC */
   
     coap_tick_t sendqueue_basetime;
     coap_queue_t *sendqueue;
     coap_endpoint_t *endpoint;      
   #ifdef WITH_POSIX
     int sockfd;                     
   #endif /* WITH_POSIX */
   
  117 #ifdef WITH_CONTIKI
  118   struct uip_udp_conn *conn;      
  119   struct etimer retransmit_timer; 
  120   struct etimer notify_timer;     
  121 #endif /* WITH_CONTIKI */

  123 #ifdef WITH_LWIP
  124   uint8_t timer_configured;       
  127 #endif /* WITH_LWIP */
   
     unsigned short message_id;
   
     unsigned int observe;
   
     coap_response_handler_t response_handler;
   //---------------------------------------------------???
     ssize_t (*network_send)(struct coap_context_t *context,
                             const coap_endpoint_t *local_interface,
                             const coap_address_t *dst,
                             unsigned char *data, size_t datalen);
   
     ssize_t (*network_read)(coap_endpoint_t *ep, coap_packet_t **packet);
  //------------------------------------------------------???
  151 } coap_context_t;
//***********************end coap_context_t***************************

  838 int
  839 coap_handle_message(coap_context_t *ctx,
  840             coap_packet_t *packet) {
  841             /* const coap_address_t *remote,  */
  842             /* unsigned char *msg, size_t msg_len) { */
  843   unsigned char *msg;
  844   size_t msg_len;
  845   coap_queue_t *node;
  846 
  847   /* the negated result code */
  848   enum result_t { RESULT_OK, RESULT_ERR_EARLY, RESULT_ERR };
  849   int result = RESULT_ERR_EARLY;
  850 
  851   coap_packet_get_memmapped(packet, &msg, &msg_len);
  852 
  853   if (msg_len < sizeof(coap_hdr_t)) {
  854     debug("coap_handle_message: discarded invalid frame\n" );
  855     goto error_early;
  856   }
  857 
  858   /* check version identifier */
  859   if (((*msg >> 6) & 0x03) != COAP_DEFAULT_VERSION) {
  860     debug("coap_handle_message: unknown protocol version %d\n", (*msg >> 6) & 0x03);
  861     goto error_early;
  862   }
  863 
  864   node = coap_new_node();
  865   if (!node) {
  866     goto error_early;
  867   }
  868 
  869   /* from this point, the result code indicates that */
  870   result = RESULT_ERR;
  871   
  872 #ifdef WITH_LWIP
  873   node->pdu = coap_pdu_from_pbuf(coap_packet_extract_pbuf(packet));
  874 #else
  875   node->pdu = coap_pdu_init(0, 0, 0, msg_len);
  876 #endif
  877   if (!node->pdu) {
  878     goto error;
  879   }
  880 
  881   if (!coap_pdu_parse(msg, msg_len, node->pdu)) {
  882     warn("discard malformed PDU\n");
  883     goto error;
  884   }
  885 
  886   coap_ticks(&node->t);
  887 
  888   coap_packet_populate_endpoint(packet, &node->local_if);
  889   coap_packet_copy_source(packet, &node->remote);
  890 
  891   /* and add new node to receive queue */
  892   coap_transaction_id(&node->remote, node->pdu, &node->id);
  893 
  894 #ifndef NDEBUG
  895   if (LOG_DEBUG <= coap_get_log_level()) {
  896 #ifndef INET6_ADDRSTRLEN
  897 #define INET6_ADDRSTRLEN 40
  898 #endif
  899 
  907     coap_show_pdu(node->pdu);
  908   }
  909 #endif
  910 
  911   coap_dispatch(ctx, node);
  912   return -RESULT_OK;
  913 
  914  error:
  915   /* FIXME: send back RST? */
  916   coap_delete_node(node);
  917   return -result;
  918 
  919  error_early:
  920   return -result;
  921 }