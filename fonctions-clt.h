#ifndef FONCTIONS_CLT_H
#define FONCTIONS_CLT_H

static void
message_handler(struct coap_context_t *ctx, const coap_endpoint_t *local_interface, 
                const coap_address_t *remote, coap_pdu_t *sent, coap_pdu_t *received, 
                const coap_tid_t id) 
{
	unsigned char* data;
	size_t         data_len;
	if (COAP_RESPONSE_CLASS(received->hdr->code) == 2) /* 2 is for success */
	{
		if (coap_get_data(received, &data_len, &data))
		{
			printf("client: Received: %s\n", data);
		}
	}
	else 
		printf("client: Not Received ! \n");
}

static void
post_handler(struct coap_context_t *ctx, const coap_endpoint_t *local_interface, 
                const coap_address_t *remote, coap_pdu_t *sent, coap_pdu_t *received, 
                const coap_tid_t id) 
{
	unsigned char* data;
	size_t         data_len;
	if (COAP_RESPONSE_CLASS(received->hdr->code) == 2){ /* 2 is for success */
		printf("client: COAP_RESPONSE_CODE: %d\n", COAP_RESPONSE_CODE(received->hdr->code));
		if (COAP_RESPONSE_CODE(received->hdr->code == 69)){
				if (coap_get_data(received, &data_len, &data)){
					printf("client: Received-Content: %s\n", data);
				}
			}
		else if (COAP_RESPONSE_CODE(received->hdr->code == 67)){
				if (coap_get_data(received, &data_len, &data)){
					printf("client: Received-Valid :%s\n ",data);
				}
			}
		else
			printf("%s\n","Unknknow COAP_RESPONSE_CODE");
	}
	else
		printf("client: Not Received ! \n");
}

#endif /*FONCTIONS_CLT_H*/
