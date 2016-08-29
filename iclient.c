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

/*256 int
  257 coap_get_data(coap_pdu_t *pdu, size_t *len, unsigned char **data) { 
  262   if (pdu->data) {
  263     *len = (unsigned char *)pdu->hdr + pdu->length - pdu->data;
  264     *data = pdu->data;
  265   } else {          // no data, clear everything /
  266     *len = 0;
  267     *data = NULL;
  268   }
  270   return *data != NULL;
  271 } */
		{
			printf("Received: %s\n", data);
		}
	}
	else 
		printf("client: Not Received ! \n");
}

int main(int argc, char* argv[])
{
	coap_context_t*   ctx;
	coap_address_t    dst_addr, src_addr;
	static coap_uri_t uri;
	fd_set            readfds; 
	coap_pdu_t*       request;
	const char*       server_uri = "coap://127.0.0.1/hello";
	unsigned char     get_method = 1;
	unsigned char     post_method = 2;
	unsigned char     put_method = 3;
	unsigned char     delete_method = 4;
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
	request            = coap_new_pdu();	
	request->hdr->type = COAP_MESSAGE_CON;
	request->hdr->id   = coap_new_message_id(ctx);
	request->hdr->code = ipatch_method; /*was : get_method*/
	coap_add_option(request, COAP_OPTION_URI_PATH, uri.path.length, uri.path.s);
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
