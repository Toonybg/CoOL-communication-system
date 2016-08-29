 coap_pdu_t *
   75 coap_pdu_init(unsigned char type, unsigned char code, 
   76           unsigned short id, size_t size) {
   77   coap_pdu_t *pdu;
   78 #ifdef WITH_LWIP
   79     struct pbuf *p;
   80 #endif
   81 
   82   assert(size <= COAP_MAX_PDU_SIZE);
   83   /* Size must be large enough to fit the header. */
   84   if (size < sizeof(coap_hdr_t) || size > COAP_MAX_PDU_SIZE)
   85     return NULL;
   86 
   87   /* size must be large enough for hdr */
   88 #if defined(WITH_POSIX) || defined(WITH_CONTIKI)
   89   pdu = coap_malloc_type(COAP_PDU, sizeof(coap_pdu_t));
   90   if (!pdu) return NULL;
   91   pdu->hdr = coap_malloc_type(COAP_PDU_BUF, size);
   92   if (pdu->hdr == NULL) {
   93     coap_free_type(COAP_PDU, pdu);
   94     pdu = NULL;
   95   }
   96 #endif /* WITH_POSIX or WITH_CONTIKI */
   97 #ifdef WITH_LWIP
   98   pdu = (coap_pdu_t*)coap_malloc_type(COAP_PDU, sizeof(coap_pdu_t));
   99   if (!pdu) return NULL;
  100   p = pbuf_alloc(PBUF_TRANSPORT, size, PBUF_RAM);
  101   if (p == NULL) {
  102     coap_free_type(COAP_PDU, pdu);
  103     pdu = NULL;
  104   }
  105 #endif
  106   if (pdu) {
  107 #ifdef WITH_LWIP
  108     pdu->pbuf = p;
  109 #endif
  110     coap_pdu_clear(pdu, size);
  111     pdu->hdr->id = id;
  112     pdu->hdr->type = type;
  113     pdu->hdr->code = code;
  114   } 
  115   return pdu;
  116 }