#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

extern "C" {
#include "vc.h"
}


int main(void) {
    // Vídeo
    cv::VideoCapture capture;
    struct
    {
        int width, height;
        int ntotalframes;
        int fps;
        int nframe;
    } video;

    std::string str;
    int key = 0;
    int nlabels;
    float circulo = 0.000;
    /* Leitura de vídeo de um ficheiro */
    /* NOTA IMPORTANTE:
    O ficheiro video.avi deverá estar localizado no mesmo diretório que o ficheiro de código fonte.
    */
    //capture.open(videofile);

    /* Em alternativa, abrir captura de vídeo pela Webcam #0 */
    capture.open(0, cv::CAP_DSHOW); // Pode-se utilizar apenas capture.open(0);

    /* Verifica se foi possível abrir o ficheiro de vídeo */
    if (!capture.isOpened())
    {
        std::cerr << "Erro ao abrir o ficheiro de vídeo!\n";
        return 1;
    }

    /* Número total de frames no vídeo */
    video.ntotalframes = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
    /* Frame rate do vídeo */
    video.fps = (int)capture.get(cv::CAP_PROP_FPS);
    /* Resolução do vídeo */
    video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
    video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

    /* Cria uma janela para exibir o vídeo */
    cv::namedWindow("VC - VIDEO", cv::WINDOW_AUTOSIZE);

    IVC* source = vc_image_new(video.width, video.height, 3, 255);
    IVC* image_rgb = vc_image_new(video.width, video.height, 3, 255);
    IVC* image = vc_image_new(video.width, video.height, 3, 255); // ORIGINAL
    IVC* imageHSV = vc_image_new(video.width, video.height, 3, 255); // HSV
    IVC* imageHSVSeg = vc_image_new(video.width, video.height, 1, 255); // HSV Segmented
    IVC* imageFinal = vc_image_new(video.width, video.height, 3, 255); // 3 Channel
    IVC* imageBlob = vc_image_new(video.width, video.height, 1, 255);
    OVC* blobs;

    cv::Mat frame;
    while (key != 'q') {

        /* Leitura de uma frame do video */
        capture.read(frame);

        /* Verifica se conseguiu ler a frame */
        if (frame.empty()) break;

        /* Numero da frame a processar */
        video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

        /* Exemplo de inserção texto na frame */
        str = std::string("RESOLUCAO: ").append(std::to_string(video.width)).append("x").append(std::to_string(video.height));
        cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
        str = std::string("TOTAL DE FRAMES: ").append(std::to_string(video.ntotalframes));
        cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
        str = std::string("FRAME RATE: ").append(std::to_string(video.fps));
        cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
        str = std::string("N. DA FRAME: ").append(std::to_string(video.nframe));
        cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);


        // Cria uma nova imagem IVC
        // 
        // Copia dados de imagem da estrutura cv::Mat para uma estrutura IVC
        memcpy(image->data, frame.data, video.width * video.height * 3);

        //Converte a imagem para RGB
        vc_convert_bgr_to_rgb(image, image_rgb);
        //Segmenta a imagem
        vc_hsv_segmentation(image_rgb, imageHSVSeg);
        //Realça as bordas
        vc_gray_highpass_filter(imageHSV, imageHSVSeg);
        //Rotula os blobs e retona o numero de blobs
        blobs = vc_binary_blob_labelling(imageHSVSeg, imageBlob, &nlabels);


        float circulo = 0.000;

        if (blobs != NULL) {
            //da a informação das blobs
            vc_binary_blob_info(imageBlob, blobs, nlabels);
        }
        // Converte a imagem binaria para 3 canais
        vc_expand_gray_to_3channels(imageBlob, imageFinal);

        // Copia dados de imagem da estrutura IVC para uma estrutura cv::Mat
        memcpy(frame.data, image->data, video.width * video.height * 3);


        for (int i = 0; i < nlabels; i++)
        {
            if (blobs[i].area > 25000 ) {
                //Desenha o circulo que representa o centro de massa do blob
                cv::circle(frame, cv::Point(blobs[i].xc, blobs[i].yc), 5, cv::Scalar(153, 51, 153), -1);
                //Desenha o retangulo que define os contornos do blob
                cv::Rect boundingRect(blobs[i].x, blobs[i].y, blobs[i].width, blobs[i].height);
                cv::rectangle(frame, boundingRect, cv::Scalar(0, 255, 0), 2);
                //Mostra as informaçoes da area e do perimetro do blob
                std::string areaBlob = std::string("Area: ").append(std::to_string(blobs[i].area));
                cv::putText(frame, areaBlob, cv::Point(blobs[i].xc, blobs[i].yc), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
                std::string perimeter = std::string("Perimeter: ").append(std::to_string(blobs[i].perimeter));
                cv::putText(frame, perimeter, cv::Point(blobs[i].xc, blobs[i].yc + 30), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
                circulo = 4.00 * 3.14 * blobs->area / (blobs->perimeter * blobs->perimeter);

                if (circulo > 0.20 && circulo < 1.70) { // Se for circular
                    int seta = detectTurnDirection(imageHSVSeg);
                    int cor = vc_hsv_segmentation(image_rgb, imageHSVSeg);
                    if (cor == 0) { //Se a cor for vermelha
                        std::string vermelho = std::string("Vermelho");
                        cv::putText(frame, vermelho, cv::Point(blobs[i].xc, blobs[i].yc + 90), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
                    }
                    else if (cor == 1) { //Se a cor for azul
                        std::string azul = std::string("Azul");
                        cv::putText(frame, azul, cv::Point(blobs[i].xc, blobs[i].yc + 90), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
                    }
                    if (seta == 0 && cor == 1) { // Se for azul e a houver mais pixeis segmentados do lado direito
                        std::string esquerda = std::string("Obrigatorio esquerda");
                        cv::putText(frame, esquerda, cv::Point(blobs[i].xc - 30, blobs[i].yc + 60), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
                    }
                    else if (seta == 1 && cor == 1) { // Se for azul e a houver mais pixeis segmentados do lado esquerdo
                        std::string direita = std::string("Obrigatorio direita");
                        cv::putText(frame, direita, cv::Point(blobs[i].xc, blobs[i].yc + 60), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
                    }
                    if (cor == 0) { // Se for vermelho
                        std::string proibido = std::string("Sentido Proibido");
                        cv::putText(frame, proibido, cv::Point(blobs[i].xc, blobs[i].yc + 60), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
                    }
                }
                else {
                    int cor = vc_hsv_segmentation(image_rgb, imageHSVSeg);
                    if (cor == 0) { // Se for vermelho e não circular
                        std::string vermelho = std::string("Vermelho");
                        cv::putText(frame, vermelho, cv::Point(blobs[i].xc, blobs[i].yc + 90), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
                        std::string stop = std::string("Sinal de STOP");
                        cv::putText(frame, stop, cv::Point(blobs[i].xc, blobs[i].yc + 60), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
                    }
                    else if (cor == 1) { // Se for azul e não circular
                        std::string azul = std::string("Azul");
                        cv::putText(frame, azul, cv::Point(blobs[i].xc, blobs[i].yc + 90), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2) ;
                    }
                }
            }

        }


        /* Exibe a frame */
        cv::imshow("VC - VIDEO", frame);

        /* Em alternativa, abrir captura de vídeo pela Webcam #0 */
        /* Sai da aplicação, se o utilizador premir a tecla 'q' */
        key = cv::waitKey(1);
    }
    //Liberta as imagens
    vc_image_free(image);
    vc_image_free(imageHSV);
    vc_image_free(image_rgb);
    vc_image_free(imageFinal);
    vc_image_free(image);
    vc_image_free(imageHSVSeg);
    vc_image_free(imageBlob);

    /* Fecha a janela */
    cv::destroyWindow("VC - VIDEO");

    /* Fecha o ficheiro de vídeo */
    capture.release();

    return 0;
}