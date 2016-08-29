    void
 	coap_dispatch(coap_context_t *context, coap_queue_t *rcvd) {
    coap_queue_t *sent = NULL;
    coap_pdu_t *response;
    coap_opt_filter_t opt_filter;
 1527 
 1528   if (!context)
 1529     return;
 1530 
 1531   memset(opt_filter, 0, sizeof(coap_opt_filter_t));
 1532 
 1533   {
 1534     /* version has been checked in coap_handle_message() */
 1535     /* if ( rcvd->pdu->hdr->version != COAP_DEFAULT_VERSION ) { */
 1536     /*   debug("dropped packet with unknown version %u\n", rcvd->pdu->hdr->version); */
 1537     /*   goto cleanup; */
 1538     /* } */
 1539     
      switch (rcvd->pdu->hdr->type) {
      case COAP_MESSAGE_ACK:
        /* find transaction in sendqueue to stop retransmission */
        coap_remove_from_queue(&context->sendqueue, rcvd->id, &sent);
  
        if (rcvd->pdu->hdr->code == 0)
      goto cleanup;
 1547 
 1548       /* if sent code was >= 64 the message might have been a 
 1549        * notification. Then, we must flag the observer to be alive
 1550        * by setting obs->fail_cnt = 0. */
 1551       if (sent && COAP_RESPONSE_CLASS(sent->pdu->hdr->code) == 2) {
 1552     const str token = 
 1553       { sent->pdu->hdr->token_length, sent->pdu->hdr->token };
 1554     coap_touch_observer(context, &sent->remote, &token);
 1555       }
 1556       break;
      case COAP_MESSAGE_RST :
        /* We have sent something the receiver disliked, so we remove
         * not only the transaction but also the subscriptions we might
         * have. */
 1563 #ifndef WITH_CONTIKI
 1564       coap_log(LOG_ALERT, "got RST for message %u\n", ntohs(rcvd->pdu->hdr->id));
 1565 #else /* WITH_CONTIKI */
 1566       coap_log(LOG_ALERT, "got RST for message %u\n", uip_ntohs(rcvd->pdu->hdr->id));
 1567 #endif /* WITH_CONTIKI */
 1568 
 1569       /* find transaction in sendqueue to stop retransmission */
 1570       coap_remove_from_queue(&context->sendqueue, rcvd->id, &sent);
 1571 
 1572       if (sent)
 1573     coap_cancel(context, sent);
 1574       goto cleanup;
 1575 
      case COAP_MESSAGE_NON : /* check for unknown critical options */
        if (coap_option_check_critical(context, rcvd->pdu, opt_filter) == 0)
      goto cleanup;
        break;
 1580 
 1581 case COAP_MESSAGE_CON : /* check for unknown critical options */
 1582       if (coap_option_check_critical(context, rcvd->pdu, opt_filter) == 0) {
 1583 
 1584     /* FIXME: send response only if we have received a request. Otherwise, 
 1585      * send RST. */
 1586     response = 
 1587       coap_new_error_response(rcvd->pdu, COAP_RESPONSE_CODE(402), opt_filter);
 1588 
 1589     if (!response)
 1590       warn("coap_dispatch: cannot create error reponse\n");
 1591     else {
 1592       if (coap_send(context, &rcvd->local_if, &rcvd->remote, response) 
 1593           == COAP_INVALID_TID) {
 1594         warn("coap_dispatch: error sending reponse\n");
 1595       }
 1596           coap_delete_pdu(response);
 1597     }    
 1598     
 1599     goto cleanup;
 1600       }
 1601     default: break;
 1602     }
 1603    
 1604     /* Pass message to upper layer if a specific handler was
 1605      * registered for a request that should be handled locally. */
 1606     if (handle_locally(context, rcvd)) {
 1607       if (COAP_MESSAGE_IS_REQUEST(rcvd->pdu->hdr))
 1608     handle_request(context, rcvd);
 1609       else if (COAP_MESSAGE_IS_RESPONSE(rcvd->pdu->hdr))
 1610     handle_response(context, sent, rcvd);
 1611       else {
 1612     debug("dropped message with invalid code (%d.%02d)\n", 
 1613           COAP_RESPONSE_CLASS(rcvd->pdu->hdr->code),
 1614           rcvd->pdu->hdr->code & 0x1f);
 1615     coap_send_message_type(context, &rcvd->local_if, &rcvd->remote, 
 1616                    rcvd->pdu, COAP_MESSAGE_RST);
 1617       }
 1618     }
 1619     
 1620   cleanup:
 1621     coap_delete_node(sent);
 1622     coap_delete_node(rcvd);
 1623   }
 1624 }