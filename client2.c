#include <coap.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

/*
 * The response handler
 */ 
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
			printf("Received: %s\n", data);
		}
	}
}

int main(int argc, char* argv[])
{
	coap_context_t*   ctx;
	coap_address_t    dst_addr, src_addr;
	static coap_uri_t uri;
	fd_set            readfds; 
	coap_pdu_t*       request;
	const char*       server_uri = "coap://127.0.0.1/c"; /*was: .../hello*/
	unsigned char     get_method = 1;
	unsigned char     fetch_method = 5;
	unsigned char     ipatch_method = 6;
	/* Prepare coap socket*/
	coap_address_init(&src_addr);
	src_addr.addr.sin.sin_family      = AF_INET;
	src_addr.addr.sin.sin_port        = htons(0);
	src_addr.addr.sin.sin_addr.s_addr = inet_addr("0.0.0.0");
	ctx = coap_new_context(&src_addr);
	/* The destination endpoint */
	coap_address_init(&dst_addr);
	dst_addr.addr.sin.sin_family      = AF_INET;
	dst_addr.addr.sin.sin_port        = htons(5683);
	dst_addr.addr.sin.sin_addr.s_addr = inet_addr("127.0.0.1");
	/* Prepare the request */
	coap_split_uri(server_uri, strlen(server_uri), &uri);
	/*TODO print uri to see what it is*/
	request            = coap_new_pdu();	
	request->hdr->type = COAP_MESSAGE_CON;
	request->hdr->id   = coap_new_message_id(ctx);
	request->hdr->code = get_method; /*was : get_method*/
	coap_add_option(request, COAP_OPTION_URI_PATH, uri.path.length, uri.path.s);
	coap_add_option(request, COAP_OPTION_URI_PATH, uri.path.length, uri.path.s); /*NEW*/
	/* Set the handler and send the request */
	
	coap_register_response_handler(ctx, message_handler);
	printf("%s at %ld\n","client: sending... at",time(NULL) );
	sleep(1);
	coap_send_confirmed(ctx, ctx->endpoint, &dst_addr, request);

	FD_ZERO(&readfds);
	FD_SET( ctx->sockfd, &readfds );
	int result = select( FD_SETSIZE, &readfds, 0, 0, NULL );
	if ( result < 0 ) /* socket error */
	{
		exit(EXIT_FAILURE);
	} 
	else if ( result > 0 && FD_ISSET( ctx->sockfd, &readfds )) /* socket read*/
	{	 
			sleep(1);
			printf("%s at %ld\n","client: coap read ...",time(NULL) );
			sleep(1);
			coap_read( ctx );
	} 
  return 0;
}
