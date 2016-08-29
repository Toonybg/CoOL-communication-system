
 1165 coap_pdu_t *
 1166 wellknown_response(coap_context_t *context, coap_pdu_t *request) {
 1167   coap_pdu_t *resp;
 1168   coap_opt_iterator_t opt_iter;
 1169   size_t len, wkc_len;
 1170   unsigned char buf[2];
 1171   int result = 0;
 1172   int need_block2 = 0;     /* set to 1 if Block2 option is required */
 1173   coap_block_t block;
 1174   coap_opt_t *query_filter;
 1175   size_t offset = 0;
 1176 
 1177   resp = coap_pdu_init(request->hdr->type == COAP_MESSAGE_CON 
 1178                ? COAP_MESSAGE_ACK 
 1179                : COAP_MESSAGE_NON,
 1180                COAP_RESPONSE_CODE(205),
 1181                request->hdr->id, COAP_MAX_PDU_SIZE);
 1182   if (!resp) {
 1183     debug("wellknown_response: cannot create PDU\n");
 1184     return NULL;
 1185   }
 1186   
 1187   if (!coap_add_token(resp, request->hdr->token_length, request->hdr->token)) {
 1188     debug("wellknown_response: cannot add token\n");
 1189     goto error;
 1190   }
 1191 
 1192   query_filter = coap_check_option(request, COAP_OPTION_URI_QUERY, &opt_iter);
 1193   wkc_len = get_wkc_len(context, query_filter);
 1194 
 1195   /* check whether the request contains the Block2 option */
 1196   if (coap_get_block(request, COAP_OPTION_BLOCK2, &block)) {
 1197     offset = block.num << (block.szx + 4);
 1198     if (block.szx > 6) {  /* invalid, MUST lead to 4.00 Bad Request */
 1199       resp->hdr->code = COAP_RESPONSE_CODE(400);
 1200       return resp;
 1201     } else if (block.szx > COAP_MAX_BLOCK_SZX) {
 1202       block.szx = COAP_MAX_BLOCK_SZX;
 1203       block.num = offset >> (block.szx + 4);
 1204     }
 1205 
 1206     need_block2 = 1;
 1207   }
 1208 
 1209   /* Check if there is sufficient space to add Content-Format option 
 1210    * and data. We do this before adding the Content-Format option to
 1211    * avoid sending error responses with that option but no actual
 1212    * content. */
 1213   if (resp->max_size <= (size_t)resp->length + 3) {
 1214     debug("wellknown_response: insufficient storage space\n");
 1215     goto error;
 1216   }
 1217 
 1218   /* Add Content-Format. As we have checked for available storage,
 1219    * nothing should go wrong here. */
 1220   assert(coap_encode_var_bytes(buf, 
 1221             COAP_MEDIATYPE_APPLICATION_LINK_FORMAT) == 1);
 1222   coap_add_option(resp, COAP_OPTION_CONTENT_FORMAT,
 1223           coap_encode_var_bytes(buf, 
 1224             COAP_MEDIATYPE_APPLICATION_LINK_FORMAT), buf);
 1225 
 1226   /* check if Block2 option is required even if not requested */
 1227   if (!need_block2 && (resp->max_size - (size_t)resp->length < wkc_len)) {
 1228     assert(resp->length <= resp->max_size);
 1229     const size_t payloadlen = resp->max_size - resp->length;
 1230     /* yes, need block-wise transfer */
 1231     block.num = 0;
 1232     block.m = 0;      /* the M bit is set by coap_write_block_opt() */
 1233     block.szx = COAP_MAX_BLOCK_SZX;
 1234     while (payloadlen < SZX_TO_BYTES(block.szx)) {
 1235       if (block.szx == 0) {
 1236     debug("wellknown_response: message to small even for szx == 0\n");
 1237     goto error;
 1238       } else {
 1239     block.szx--;
 1240       }
 1241     }
 1242 
 1243     need_block2 = 1;
 1244   }
 1245 
 1246   /* write Block2 option if necessary */
 1247   if (need_block2) {
 1248     if (coap_write_block_opt(&block, COAP_OPTION_BLOCK2, resp, wkc_len) < 0) {
 1249       debug("wellknown_response: cannot add Block2 option\n");
 1250       goto error;
 1251     }
 1252   }
 1253 
 1254   /* Manually set payload of response to let print_wellknown() write,
 1255    * into our buffer without copying data. */
 1256 
 1257   resp->data = (unsigned char *)resp->hdr + resp->length;
 1258   *resp->data = COAP_PAYLOAD_START;
 1259   resp->data++;
 1260   resp->length++;
 1261   len = need_block2 ? SZX_TO_BYTES(block.szx) : resp->max_size - resp->length;
 1262 
 1263   result = print_wellknown(context, resp->data, &len, offset, query_filter);
 1264   if ((result & COAP_PRINT_STATUS_ERROR) != 0) {
 1265     debug("print_wellknown failed\n");
 1266     goto error;
 1267   } 
 1268   
 1269   resp->length += COAP_PRINT_OUTPUT_LENGTH(result);
 1270   return resp;
 1271 
 1272  error:
 1273   /* set error code 5.03 and remove all options and data from response */
 1274   resp->hdr->code = COAP_RESPONSE_CODE(503);
 1275   resp->length = sizeof(coap_hdr_t) + resp->hdr->token_length;
 1276   return resp;
 1277 }