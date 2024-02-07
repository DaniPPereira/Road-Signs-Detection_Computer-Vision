// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE
//                          2022/2023
//             ENGENHARIA DE SISTEMAS INFORMÁTICOS
//                    VISÃO POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include "vc.h"
#include <math.h>




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar memória para uma imagem
IVC* vc_image_new(int width, int height, int channels, int levels)
{
	IVC* image = (IVC*)malloc(sizeof(IVC));

	if (image == NULL)
		return NULL;
	if ((levels <= 0) || (levels > 255))
		return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char*)malloc(image->width * image->height * image->channels * sizeof(char));

	if (image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}



// Libertar memória de uma imagem
IVC* vc_image_free(IVC* image)
{
	if (image != NULL)
	{
		if (image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//trabalho
int vc_hsv_segmentation(IVC* src, IVC* dst, OVC* blobs, int nblobs) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	float max, min, hue, sat, valor;
	long int pos_src, pos_dst;
	float rf, gf, bf;
	int isRed = 0;
	int isBlue = 0;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 1)) return 0;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			rf = (float)datasrc[pos_src];
			gf = (float)datasrc[pos_src + 1];
			bf = (float)datasrc[pos_src + 2];

			max = rf;
			if (gf > max) max = gf;
			if (bf > max) max = bf;

			min = rf;
			if (gf < min) min = gf;
			if (bf < min) min = bf;

			valor = max;

			if (max == 0 || max == min) {
				sat = 0;
				hue = 0;
			}
			else {
				sat = (max - min) * 100.0f / valor;
				if (rf == max && gf >= bf) {
					hue = 60.0f * (gf - bf) / (max - min);
				}
				else if (rf == max && bf > gf) {
					hue = 360 + 60.0f * (gf - bf) / (max - min);
				}
				else if (gf == max) {
					hue = 120 + 60.0f * (bf - rf) / (max - min);
				}
				else if (max == bf) {
					hue = 240 + 60.0f * (rf - gf) / (max - min);
				}
			}

			if (((hue >= 0 && hue <= 20) || (hue >= 340 && hue <= 360)) && sat >= 60 && valor >= 60) {
				datadst[pos_dst] = 255; // Vermelho
				isRed = 1;
			}
			else if ((hue >= 200 && hue <= 260) && sat >= 60 && valor >= 60) {
				datadst[pos_dst] = 255; // Azul
				isBlue = 1;
			}
			else
				datadst[pos_dst] = 0;
		}
	}

	if (isRed)
		return 0;
	else if (isBlue)
		return 1;
	else
		return -1; // Neither red nor blue
}



void vc_convert_bgr_to_rgb(IVC* src, IVC* dst) {
	int width = src->width;
	int height = src->height;

	if ((src->width <= 0) || (src->height <= 0)) return;
	if ((src->width != dst->width) || (src->height != dst->height)) return;
	if ((src->channels != 3) || (dst->channels != 3)) return;

	int srcBytesPerPixel = src->channels;
	int dstBytesPerPixel = dst->channels;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int srcPos = y * src->bytesperline + x * srcBytesPerPixel;
			int dstPos = y * dst->bytesperline + x * dstBytesPerPixel;

			dst->data[dstPos] = src->data[srcPos + 2];     // R
			dst->data[dstPos + 1] = src->data[srcPos + 1]; // G
			dst->data[dstPos + 2] = src->data[srcPos];     // B
		}
	}
}

void vc_expand_gray_to_3channels(const IVC* src, IVC* dst) {
	int width = src->width;
	int height = src->height;

	if ((src->width <= 0) || (src->height <= 0)) return;
	if ((src->width != dst->width) || (src->height != dst->height) || (dst->channels != 3)) return;

	int srcBytesPerPixel = src->channels;
	int dstBytesPerPixel = dst->channels;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int srcPos = y * src->bytesperline + x * srcBytesPerPixel;
			int dstPos = y * dst->bytesperline + x * dstBytesPerPixel;

			unsigned char pixel = src->data[srcPos];

			dst->data[dstPos] = pixel;   // Channel 1
			dst->data[dstPos + 1] = pixel; // Channel 2
			dst->data[dstPos + 2] = pixel;  // Channel 3
		}
	}
}



int vc_gray_highpass_filter(IVC* src, IVC* dst) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int x, y;
	int soma;
	long int pos_src, pos_dst;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 1))return 0;

	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			pos_dst = y * bytesperline_dst + x * channels_dst;


			soma = datasrc[(y - 1) * bytesperline_src + (x - 1) * channels_src] * (-1);
			soma += datasrc[(y - 1) * bytesperline_src + (x)*channels_src] * (-2);
			soma += datasrc[(y - 1) * bytesperline_src + (x + 1) * channels_src] * (-1);
			soma += datasrc[(y)*bytesperline_src + (x - 1) * channels_src] * (-2);
			soma += datasrc[(y)*bytesperline_src + (x + 1) * channels_src] * (-2);
			soma += datasrc[(y + 1) * bytesperline_src + (x - 1) * channels_src] * (-1);
			soma += datasrc[(y + 1) * bytesperline_src + (x)*channels_src] * (-2);
			soma += datasrc[(y + 1) * bytesperline_src + (x + 1) * channels_src] * (-1);
			soma += datasrc[pos_dst] * (12);

			soma = (unsigned char)MIN(MAX((float)datasrc[pos_dst] + ((float)soma / (float)16) * 6.0, 0), 255);

			datadst[pos_dst] = soma;

		}
	}

	return 1;
}

OVC* vc_binary_blob_labelling(IVC* src, IVC* dst, int* nlabels)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int* labeltable = (int*)calloc(width * height, sizeof(int)); // Array para mapear etiquetas
	int label = 1; // Etiqueta inicial.
	int num, tmplabel;
	OVC* blobs; // Apontador para array de blobs (objetos) que será retornado desta função.

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return NULL;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem binária para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pixéis de plano de fundo devem obrigatoriamente ter valor 0
	// Todos os pixéis de primeiro plano devem obrigatoriamente ter valor 255
	// Serão atribuídas etiquetas no intervalo [1, width*height - 1]
	for (i = 0, size = bytesperline * height; i < size; i++)
	{
		if (datadst[i] != 0) datadst[i] = 255;
	}

	// Limpa os rebordos da imagem binária
	for (y = 0; y < height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}
	for (x = 0; x < width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

			posA = (y - 1) * bytesperline + (x - 1) * channels; // A
			posB = (y - 1) * bytesperline + x * channels; // B
			posC = (y - 1) * bytesperline + (x + 1) * channels; // C
			posD = y * bytesperline + (x - 1) * channels; // D
			posX = y * bytesperline + x * channels; // X

			// Se o pixel foi marcado
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				else
				{
					num = width * height;

					// Se A está marcado
					if (datadst[posA] != 0) num = labeltable[datadst[posA]];
					// Se B está marcado, e é menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
					// Se C está marcado, e é menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
					// Se D está marcado, e é menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num)) num = labeltable[datadst[posD]];

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Atualiza a tabela de etiquetas
					if (datadst[posA] != 0)
					{
						if (labeltable[datadst[posA]] != num)
						{
							for (tmplabel = labeltable[datadst[posA]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posB] != 0)
					{
						if (labeltable[datadst[posB]] != num)
						{
							for (tmplabel = labeltable[datadst[posB]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posC] != 0)
					{
						if (labeltable[datadst[posC]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posD] != 0)
					{
						if (labeltable[datadst[posD]] != num)
						{
							for (tmplabel = labeltable[datadst[posD]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
				}
			}
		}
	}



	// Contagem do número de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a < label - 1; a++)
	{
		for (b = a + 1; b < label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que não hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a < label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se não há blobs
	if (*nlabels == 0) return NULL;

	// Cria lista de blobs (objetos) e preenche a etiqueta
	blobs = (OVC*)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a < (*nlabels); a++) blobs[a].label = labeltable[a];
	}
	else return NULL;

	return blobs;
}



int vc_binary_blob_info(IVC* src, OVC* blobs, int nblobs)
{
	unsigned char* data = (unsigned char*)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	// Conta �rea de cada blob
	for (i = 0; i < nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		blobs[i].area = 0;

		for (y = 1; y < height - 1; y++)
		{
			for (x = 1; x < width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// �rea
					blobs[i].area++;

					// Centro de Gravidade
					sumx += x;
					sumy += y;

					// Bounding Box
					if (xmin > x) xmin = x;
					if (ymin > y) ymin = y;
					if (xmax < x) xmax = x;
					if (ymax < y) ymax = y;

					// Per�metro
					// Se pelo menos um dos quatro vizinhos n�o pertence ao mesmo label, ent�o � um pixel de contorno
					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
					{
						blobs[i].perimeter++;
					}
				}
			}
		}

		// Bounding Box
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;

		blobs[i].xc = sumx / MAX(blobs[i].area, 1);
		blobs[i].yc = sumy / MAX(blobs[i].area, 1);
	}

	return 1;
}

int detectTurnDirection(const IVC* imageSeg) {
	int width = imageSeg->width;
	int height = imageSeg->height;
	int halfWidth = width / 2;
	int rightCount = 0;
	int leftCount = 0;

	unsigned char* data = (unsigned char*)imageSeg->data;
	int bytesperline = imageSeg->bytesperline;
	int channels = imageSeg->channels;

	// Conta os pixels segmentados no lado direito e esquerdo da imagem
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			long int pos = y * bytesperline + x * channels;

			if (data[pos] != 0) {
				if (x < halfWidth) {
					leftCount++;
				}
				else {
					rightCount++;
				}
			}
		}
	}

	// Verifica qual lado tem mais pixels segmentados e imprime a mensagem correspondente
	if (rightCount > leftCount) {
		return 0;
	}
	else {
		return 1;
	}
}

