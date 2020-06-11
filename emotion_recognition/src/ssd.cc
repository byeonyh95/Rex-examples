#include "ssd.h"

char EMOTIONS[NUM_CLASS][10] = { "angry", "disgust", "fearful", "happy", "sad", "surprised", "neutral" }; // 7���� ǥ��


void ssd_post(float* outputs, int w, int h, ssd_ctx* ctx)
{
	int i;
	float *emotion = outputs; // rknn output

	// max score print
	for (i=0; i<NUM_CLASS; i++) {// 7���� ǥ�� �� ���� ���� score�� ǥ�� �� 1�� ����
		if (emotion[i] == 1) {// 1 ���� ���� ǥ�� string ���
			printf(">> %s\n", EMOTIONS[i]);
		}
	}
}
int ssd_run(ssd_ctx *ctx, uint8_t *buff, int w, int h, ssize_t size)
{
	int ret, i, j=0;
	float gray;
	rknn_input inputs[1];
	rknn_output outputs[1];
	uint8_t img[size];
	
/*
	for(i=0; i < size * 3; i+=3){
		gray = 0;
		gray += buff[i];
		gray += buff[i+1];
		gray += buff[i+2];
		img[j] = (uint8_t)(gray/3.0);
		j++;
	}	
*/
	memset(inputs, 0x00, sizeof(inputs));
	inputs[0].index = 0;
	inputs[0].type = RKNN_TENSOR_UINT8;
	inputs[0].size = size;
	inputs[0].fmt = RKNN_TENSOR_NHWC;
	//inputs[0].buf = img;
	inputs[0].buf = buff;


	//printf("input set start \n");
	ret = rknn_inputs_set(ctx->rknn, 1, inputs);
	if (ret < 0) {
		fprintf(stdout, "%s fail\n", __func__);
		return -1;
	}
	//printf("input set \n");
	ret = rknn_run(ctx->rknn, NULL);
	if (ret < 0) {
		fprintf(stdout, "%s fail\n", __func__);
		return -1;
	}

	//printf("rknn_run done\n");

	memset(outputs, 0x00, sizeof(outputs));
	outputs[0].want_float = 1;

	ret = rknn_outputs_get(ctx->rknn, 1, outputs, NULL);
	if (ret < 0) {
		fprintf(stdout, "%s fail\n", __func__);
		return -1;
	}

	//printf("get output done\n");
	//SSD ��ó���ڵ�
	ssd_post((float *)outputs[0].buf, w, h, ctx);
	rknn_outputs_release(ctx->rknn, 1, outputs);

	return 0;
}


char *readLine(FILE *fp, char *buffer, int *len) 
{
    int ch;
    int i = 0;
    size_t buff_len = 0;

    buffer = (char *)malloc(buff_len + 1);
    if (!buffer)
        return NULL; // Out of memory

    while ((ch = fgetc(fp)) != '\n' && ch != EOF) {
        buff_len++;
        void *tmp = realloc(buffer, buff_len + 1);
        if (tmp == NULL) {
            free(buffer);
            return NULL; // Out of memory
        }
        buffer = (char *)tmp;

        buffer[i] = (char)ch;
        i++;
    }
    buffer[i] = '\0';

    *len = buff_len;

    // Detect end
    if (ch == EOF && (i == 0 || ferror(fp))) {
        free(buffer);
        return NULL;
    }
    return buffer;
}

int readLines(const char *fileName, char *lines[]) 
{
    FILE *file = fopen(fileName, "r");
    char *s;
    int i = 0;
    int n = 0;
    while ((s = readLine(file, s, &n)) != NULL) {
        lines[i++] = s;
    }
    return i;
}

int loadLabelName(const char *locationFilename, char *labels[]) 
{
    readLines(locationFilename, labels);
    return 0;
}




int ssd_init(const char *model_name, ssd_ctx *ctx)
{
	int ret;

	ret = rknn_init_helper(model_name, &ctx->rknn);
	if (ret != 0) {
		fprintf(stderr, "%s : Failed to load model", __func__);
		return -1;
	}
	return 0;
}

