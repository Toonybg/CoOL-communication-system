 int
   coap_handle_message(coap_context_t *ctx,
               coap_packet_t *packet) {
               /* const coap_address_t *remote,  */
               /* unsigned char *msg, size_t msg_len) { */
     unsigned char *msg;
     size_t msg_len;
     coap_queue_t *node; //!!! node is a coap_queue_t type
   
     /* the negated result code */
     enum result_t { RESULT_OK, RESULT_ERR_EARLY, RESULT_ERR };
     int result = RESULT_ERR_EARLY;
   
     coap_packet_get_memmapped(packet, &msg, &msg_len);
     /*msg have the address of the received packet*/
   
     if (msg_len < sizeof(coap_hdr_t)) {
       debug("coap_handle_message: discarded invalid frame\n" );
       goto error_early;
     }
   
     /* check version identifier */
     if (((*msg >> 6) & 0x03) != COAP_DEFAULT_VERSION) {
       debug("coap_handle_message: unknown protocol version %d\n", (*msg >> 6) & 0x03);
       goto error_early;
     }
   
     node = coap_new_node();
     if (!node) {
       goto error_early;
     }
   
     /* from this point, the result code indicates that */
     result = RESULT_ERR;
  
    /* TODO -- GDB -- check content here -- msg,msg_len,node->pdu */
   /*parses data into the coap pdu structure -- node->pdu*/
     if (!coap_pdu_parse(msg, msg_len, node->pdu)) {
       warn("discard malformed PDU\n");
       goto error;
     }
   
  886   coap_ticks(&node->t);
  887 
  888   coap_packet_populate_endpoint(packet, &node->local_if);
  889   coap_packet_copy_source(packet, &node->remote);
  890   /* Given an incoming packet, copy its source address into an address struct*/
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
      // the packet is read/processed/interpreted in coap_dispatch 

      //node has the receied information --
       coap_dispatch(ctx, node);


  //-------------------------------------
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