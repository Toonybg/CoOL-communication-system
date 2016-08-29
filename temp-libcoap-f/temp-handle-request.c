static void
 1340 handle_request(coap_context_t *context, coap_queue_t *node) {      
 1341   coap_method_handler_t h = NULL;
 1342   coap_pdu_t *response = NULL;
 1343   coap_opt_filter_t opt_filter;
 1344   coap_resource_t *resource;
 1345   coap_key_t key;
 1346 
 1347   coap_option_filter_clear(opt_filter);
 1348   
 1349   /* try to find the resource from the request URI */
 1350   coap_hash_request_uri(node->pdu, key);
 1351   resource = coap_get_resource_from_key(context, key);
 1352   
 1353   if (!resource) {
 1354     /* The resource was not found. Check if the request URI happens to
 1355      * be the well-known URI. In that case, we generate a default
 1356      * response, otherwise, we return 4.04 */
 1357 
 1358     switch(node->pdu->hdr->code) {
 1359 
 1360     case COAP_REQUEST_GET: 
 1361       if (is_wkc(key)) {    /* GET request for .well-known/core */
 1362     info("create default response for %s\n", COAP_DEFAULT_URI_WELLKNOWN);
 1363     response = coap_wellknown_response(context, node->pdu);
 1364 
 1365       } else { /* GET request for any another resource, return 4.04 */
 1366 
 1367     debug("GET for unknown resource 0x%02x%02x%02x%02x, return 4.04\n", 
 1368           key[0], key[1], key[2], key[3]);
 1369     response = 
 1370       coap_new_error_response(node->pdu, COAP_RESPONSE_CODE(404), 
 1371                   opt_filter);
 1372       }
 1373       break;
 1374 
 1375     default:            /* any other request type */
 1376 
 1377       debug("unhandled request for unknown resource 0x%02x%02x%02x%02x\r\n",
 1378         key[0], key[1], key[2], key[3]);
 1379       if (!coap_mcast_interface(&node->local_if))
 1380     response = coap_new_error_response(node->pdu, COAP_RESPONSE_CODE(405), 
 1381                        opt_filter);
 1382     }
 1383       
 1384     if (response && !no_response(node->pdu, response) && coap_send(context, &node->local_if, 
 1385                   &node->remote, response) == COAP_INVALID_TID) {
 1386       warn("cannot send response for transaction %u\n", node->id);
 1387     }
 1388     coap_delete_pdu(response);
 1389 
 1390     return;
 1391   }
 1392   
 1393   /* the resource was found, check if there is a registered handler */
 1394   if ((size_t)node->pdu->hdr->code - 1 <
 1395       sizeof(resource->handler)/sizeof(coap_method_handler_t))
 1396     h = resource->handler[node->pdu->hdr->code - 1];
 1397   
 1398   if (h) {
 1399     debug("call custom handler for resource 0x%02x%02x%02x%02x\n", 
 1400       key[0], key[1], key[2], key[3]);
 1401     response = coap_pdu_init(node->pdu->hdr->type == COAP_MESSAGE_CON 
 1402                  ? COAP_MESSAGE_ACK
 1403                  : COAP_MESSAGE_NON,
 1404                  0, node->pdu->hdr->id, COAP_MAX_PDU_SIZE);
 1405     
 1406     /* Implementation detail: coap_add_token() immediately returns 0
 1407        if response == NULL */
 1408     if (coap_add_token(response, node->pdu->hdr->token_length,
 1409                node->pdu->hdr->token)) {
 1410       str token = { node->pdu->hdr->token_length, node->pdu->hdr->token };
 1411       coap_opt_iterator_t opt_iter;
 1412       coap_opt_t *observe = NULL;
 1413       int observe_action = COAP_OBSERVE_CANCEL;
 1414 
 1415       /* check for Observe option */
 1416       if (resource->observable) {
 1417     observe = coap_check_option(node->pdu, COAP_OPTION_OBSERVE, &opt_iter);
 1418     if (observe) {
 1419       observe_action =
 1420         coap_decode_var_bytes(coap_opt_value(observe),
 1421                   coap_opt_length(observe));
 1422      
 1423       if ((observe_action & COAP_OBSERVE_CANCEL) == 0) {
 1424         coap_subscription_t *subscription;
 1425 
 1426         coap_log(LOG_DEBUG, "create new subscription\n");
 1427         subscription = coap_add_observer(resource, &node->local_if, 
 1428                          &node->remote, &token);
 1429         if (subscription) {
 1430           subscription->non = node->pdu->hdr->type == COAP_MESSAGE_NON;
 1431           coap_touch_observer(context, &node->remote, &token);
 1432         }
 1433       }
 1434     }
 1435       }
 1436 
 1437       h(context, resource, &node->local_if, &node->remote,
 1438     node->pdu, &token, response);
 1439 
 1440       if (!no_response(node->pdu, response)) {
 1441       if (observe && ((COAP_RESPONSE_CLASS(response->hdr->code) > 2)
 1442               || ((observe_action & COAP_OBSERVE_CANCEL) != 0))) {
 1443     coap_log(LOG_DEBUG, "removed observer");
 1444     coap_delete_observer(resource,  &node->remote, &token);
 1445       }
 1446 
 1447       /* If original request contained a token, and the registered
 1448        * application handler made no changes to the response, then
 1449        * this is an empty ACK with a token, which is a malformed
 1450        * PDU */
 1451       if ((response->hdr->type == COAP_MESSAGE_ACK)
 1452       && (response->hdr->code == 0)) {
 1453     /* Remove token from otherwise-empty acknowledgment PDU */
 1454     response->hdr->token_length = 0;
 1455     response->length = sizeof(coap_hdr_t);
 1456       }
 1457 
 1458       if (response->hdr->type != COAP_MESSAGE_NON ||
 1459       (response->hdr->code >= 64 
 1460        && !coap_mcast_interface(&node->local_if))) {
 1461 
 1462     if (coap_send(context, &node->local_if, 
 1463               &node->remote, response) == COAP_INVALID_TID) {
 1464       debug("cannot send response for message %d\n", node->pdu->hdr->id);
 1465       }
 1466       }
 1467       }
 1468       coap_delete_pdu(response);
 1469     } else {
 1470       warn("cannot generate response\r\n");
 1471     }
 1472   } else {
 1473     if (WANT_WKC(node->pdu, key)) {
 1474       debug("create default response for %s\n", COAP_DEFAULT_URI_WELLKNOWN);
 1475       response = coap_wellknown_response(context, node->pdu);
 1476       debug("have wellknown response %p\n", (void *)response);
 1477     } else
 1478       response = coap_new_error_response(node->pdu, COAP_RESPONSE_CODE(405), 
 1479                      opt_filter);
 1480     
 1481     if (!response || (coap_send(context, &node->local_if, &node->remote,
 1482                 response) == COAP_INVALID_TID)) {
 1483       debug("cannot send response for transaction %u\n", node->id);
 1484     }
 1485     coap_delete_pdu(response);
 1486   }  
 1487 }