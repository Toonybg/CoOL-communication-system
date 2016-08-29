#ifndef FONCTIONS_H
#define FONCTIONS_H

static void
hello_handler(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response) 
{
	unsigned char buf[3];
	const char* response_data     = "Hello World! - new data";
	response->hdr->code           = COAP_RESPONSE_CODE(205);
	coap_add_option(response, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, COAP_MEDIATYPE_APPLICATION_CBOR), buf);
	coap_add_data  (response, strlen(response_data), (unsigned char *)response_data);
	switch (request->hdr->code) {
		case COAP_REQUEST_GET:
			printf("GET_REQUEST...\n");
			response->hdr->code           = COAP_RESPONSE_CODE(205);
			coap_add_option(response, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, COAP_MEDIATYPE_APPLICATION_CBOR), buf);
			coap_add_data  (response, strlen(response_data), (unsigned char *)response_data);
		break;
		case COAP_REQUEST_POST:
			printf("POST_REQUEST...\n");
		break;
		
		case COAP_REQUEST_PUT:
		    printf("PUT_REQUEST...\n");
		break;
		
		case COAP_REQUEST_DELETE:
			printf("DELETE_REQUEST...\n");
		break;
		
		case COAP_REQUEST_FETCH:
			printf("FETCH_REQUEST...\n");
		break;
		
		case COAP_REQUEST_PATCH:
			printf("PATCH_REQUEST...\n");
		break;
		
		default:
			printf("hello_handler: request->header->code error\n");
	}
}

static void 
c_handler(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response) 
{
	unsigned char buf[3];
	const char* response_data     = "new resource /c";
	switch (request->hdr->code) {
		case COAP_REQUEST_GET:
			printf("GET_REQUEST...\n");
			response->hdr->code           = COAP_RESPONSE_CODE(205);
			coap_add_option(response, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, COAP_MEDIATYPE_APPLICATION_CBOR), buf);
			coap_add_data  (response, strlen(response_data), (unsigned char *)response_data);
		break;
		case COAP_REQUEST_POST:
			printf("POST_REQUEST...\n");
		break;
		
		case COAP_REQUEST_PUT:
		    printf("PUT_REQUEST...\n");
		break;
		
		case COAP_REQUEST_DELETE:
			printf("DELETE_REQUEST...\n");
		break;
		
		case COAP_REQUEST_FETCH:
			printf("FETCH_REQUEST...\n");
		break;
		
		case COAP_REQUEST_PATCH:
			printf("PATCH_REQUEST...\n");
			
		break;
		
		default:
			printf("hello_handler: request->header->code error\n");
		}
} /*NEW*/


#endif /*FONCTIONS_H*/
