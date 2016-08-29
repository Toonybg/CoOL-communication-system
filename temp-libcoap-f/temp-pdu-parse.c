  int
   coap_pdu_parse(unsigned char *data, size_t length, coap_pdu_t *pdu) {
     coap_opt_t *opt;
   
     assert(data);
     assert(pdu);
   
     if (pdu->max_size < length) {
       debug("insufficient space to store parsed PDU\n");
       return 0;
     }
   
     if (length < sizeof(coap_hdr_t)) {
       debug("discarded invalid PDU\n");
     }
   
   #ifdef WITH_LWIP
     LWIP_ASSERT("coap_pdu_parse with unexpected addresses", data == pdu->hdr);
     LWIP_ASSERT("coap_pdu_parse with unexpected length", length == pdu->length);
   #else
   
     pdu->hdr->version = data[0] >> 6;
     pdu->hdr->type = (data[0] >> 4) & 0x03;
     pdu->hdr->token_length = data[0] & 0x0f;
     pdu->hdr->code = data[1];
   #endif
     pdu->data = NULL;
   
     /* sanity checks */
     if (pdu->hdr->code == 0) {
       if (length != sizeof(coap_hdr_t) || pdu->hdr->token_length) {
         debug("coap_pdu_parse: empty message is not empty\n");
         goto discard;
       }
     }
   
     if (length < sizeof(coap_hdr_t) + pdu->hdr->token_length
         || pdu->hdr->token_length > 8) {
       debug("coap_pdu_parse: invalid Token\n");
       goto discard;
     }
   
   #ifndef WITH_LWIP
  383   /* Copy message id in network byte order, so we can easily write the
  384    * response back to the network. */
  385   memcpy(&pdu->hdr->id, data + 2, 2);
  386 
  387   /* append data (including the Token) to pdu structure */
  388   memcpy(pdu->hdr + 1, data + sizeof(coap_hdr_t), length - sizeof(coap_hdr_t));
  389   pdu->length = length;
  390  
  391   /* Finally calculate beginning of data block and thereby check integrity
  392    * of the PDU structure. */
   #endif
  394 
  395   /* skip header + token */
  396   length -= (pdu->hdr->token_length + sizeof(coap_hdr_t));
  397   opt = (unsigned char *)(pdu->hdr + 1) + pdu->hdr->token_length;
  398 
  399   while (length && *opt != COAP_PAYLOAD_START) {
  400     if (!next_option_safe(&opt, (size_t *)&length)) {
  401       debug("coap_pdu_parse: drop\n");
  402       goto discard;
  403     }
  404   }
  405 
  406   /* end of packet or start marker */
  407   if (length) {
  408     assert(*opt == COAP_PAYLOAD_START);
  409     opt++; length--;
  410 
  411     if (!length) {
  412       debug("coap_pdu_parse: message ending in payload start marker\n");
  413       goto discard;
  414     }
  415 
  416     debug("set data to %p (pdu ends at %p)\n", (unsigned char *)opt, 
  417       (unsigned char *)pdu->hdr + pdu->length);
  418     pdu->data = (unsigned char *)opt;
  419   }
  420 
  421   return 1;
  422 
  423  discard:
  424   return 0;
  425 }