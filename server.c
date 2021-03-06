#include <coap.h>
#include <stdio.h>
#include <time.h>
#include "fonctions-srv.h"

/*
 * The resource handler
 */ 

int main(int argc, char* argv[])
{
	coap_context_t*  ctx;
	coap_address_t   serv_addr;
	coap_resource_t* hello_resource;
	coap_resource_t* c_resource; /*NEW*/
	fd_set           readfds;    
	/* Prepare the CoAP server socket */ 
	coap_address_init(&serv_addr);	
	serv_addr.addr.sin.sin_family      = AF_INET;
	serv_addr.addr.sin.sin_addr.s_addr = INADDR_ANY;
	serv_addr.addr.sin.sin_port        = htons(5683); //default port
	ctx                                = coap_new_context(&serv_addr); //a c_new_ctx returns-> 1 if OK
	if (!ctx) exit(EXIT_FAILURE);
	/* Initialize the hello resource */
	hello_resource = coap_resource_init((unsigned char *)"hello", 5, 0);
	c_resource = coap_resource_init((unsigned char *)"c", 1, 0); /*NEW*/
	coap_register_handler(hello_resource, COAP_REQUEST_GET, hello_handler);
	coap_register_handler(hello_resource, COAP_REQUEST_POST, hello_handler);
	coap_register_handler(c_resource, COAP_REQUEST_GET, c_handler); /*NEW*/
	coap_register_handler(c_resource, COAP_REQUEST_POST, c_handler); /*NEW*/
	coap_register_handler(c_resource, COAP_REQUEST_PUT, c_handler); /*NEW*/
	coap_register_handler(c_resource, COAP_REQUEST_DELETE, c_handler); /*NEW*/
	coap_register_handler(c_resource, COAP_REQUEST_FETCH, c_handler); /*NEW*/
	coap_register_handler(c_resource, COAP_REQUEST_PATCH, c_handler); /*NEW*/
	coap_add_resource(ctx, hello_resource);
	coap_add_resource(ctx, c_resource); /*NEW*/
	/*Listen for incoming connections*/
	while (1) {
		FD_ZERO(&readfds); /* initialize file descripor set to empty*/ 
		
		FD_SET( ctx->sockfd, &readfds ); /*adds the ctxt sockfd to readfds*/
		int result = select( FD_SETSIZE, &readfds, 0, 0, NULL ); /*Null i.e no timeout condition*/
		if ( result < 0 ) /* socket error */
		{
			exit(EXIT_FAILURE);
		} 
		else if ( result > 0 && FD_ISSET( ctx->sockfd, &readfds )) /* socket read*/
		{	 
				printf("%s at %ld\n","server: coap read ...",time(NULL) );
				coap_read( ctx ); /*sends the response*/
		} 
	}    
}