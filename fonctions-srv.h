#ifndef FONCTIONS_SRV_H
#define FONCTIONS_SRV_H

#include <stdlib.h>

int sidToName(int sid){
	if (sid == 10801){
		return 10801;
	}
	else
		return 0;
}

const char * sidToName2(int sid){
	printf("sid2Name sid : %d\n", sid );
	if (sid == 10801){
		return "/time";
	}
	else
		return "unknown sid";
}

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
		case COAP_REQUEST_POST:	printf("server: POST_REQUEST\n"); break;
		
		case COAP_REQUEST_PUT: printf("server: PUT_REQUEST\n"); break;
		
		case COAP_REQUEST_DELETE: printf("server: DELETE_REQUEST\n"); break;
		
		case COAP_REQUEST_FETCH: printf("server: FETCH_REQUEST\n"); break;
		
		case COAP_REQUEST_PATCH: printf("server: PATCH_REQUEST\n"); break;
		
		default: printf("hello_handler: request->header->code error\n");
	}
}

static void 
c_handler(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response) 
{
	unsigned char buf[3];
	const char* response_data;
	switch (request->hdr->code) {
		case COAP_REQUEST_GET:
			printf("GET_REQUEST...\n");
			response->hdr->code           = COAP_RESPONSE_CODE(205);
			response_data     = "new resource /c";
			coap_add_option(response, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, COAP_MEDIATYPE_APPLICATION_CBOR), buf);
			coap_add_data  (response, strlen(response_data), (unsigned char *)response_data);
				break;
		case COAP_REQUEST_POST:
			printf("server: POST_REQUEST\n"); 
			response->hdr->code           = COAP_RESPONSE_CODE(203);
			{ /*data read*/
			unsigned char* data;
			size_t         data_len;
			if (coap_get_data(request, &data_len, &data)){
					printf("server: Received-Data: %s\n ",data);
					response_data     = sidToName2(atoi(data));
					printf("server: response_data: %s\n", response_data);
				}
			}
			
			coap_add_option(response, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, COAP_MEDIATYPE_APPLICATION_CBOR), buf);
			coap_add_data  (response, strlen(response_data), (unsigned char *)response_data);
				break;

		case COAP_REQUEST_PUT: printf("server: PUT_REQUEST\n"); break;
		
		case COAP_REQUEST_DELETE: printf("server: DELETE_REQUEST\n"); break;
		
		case COAP_REQUEST_FETCH: printf("server: FETCH_REQUEST\n"); break;
		
		case COAP_REQUEST_PATCH: printf("server: PATCH_REQUEST\n"); break;
		
		default: printf("hello_handler: request->header->code error\n");
	}
} /*NEW*/


#endif /*FONCTIONS_SRV_H*/
