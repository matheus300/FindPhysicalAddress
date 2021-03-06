#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PAGEMAP_LENGTH 8
#define PAGE_SHIFT 12

/*
   Funcao responsavel por encontrar o endereco do frame em que se encontra o ponteiro passado como parametro
*/
unsigned long getNumeroPageFrame(void *addr) {
	// le o arquivo que armazena o endereco fisico dos frames de cada página mapeada
	FILE *pagemap = fopen("/proc/self/pagemap", "rb");
	// Pega apenas o offset do enderecamento
	// Para isso, divide-se o endereco pelo tamanho de pagina do sistema (4096). 
	// Multiplica-se por 8
	unsigned long offset = (unsigned long) addr / getpagesize() * PAGEMAP_LENGTH;
	// anda com o ponteiro para a posição do quadro no arquivo pagemap
	fseek(pagemap, (unsigned long)offset, SEEK_SET);

	// le os 56 bits que representam os bytes de enderecamento e apaga o 55th bit
	unsigned long page_frame_number = 0;
	fread(&page_frame_number, 1, PAGEMAP_LENGTH-1, pagemap);
	page_frame_number &= 0x7FFFFFFFFFFFFF;            // elimina-se o 55th bit 

	fclose(pagemap);
	return page_frame_number;
}

int main () {
   int retval, aux = 1;
   int *ptr;
   ptr = &aux;
   retval = fork();
   if (retval < 0) {
      perror ("Erro");
      exit (1) ;
   } else {
      if (retval > 0) {
         wait(0);
         printf("\nPai: \n");
         printf("Endereco Logico: %p\n", ptr);
         unsigned long int enderecoQuadro = getNumeroPageFrame(ptr);
         printf("Endereco de quadro: %lu\n", enderecoQuadro);
         // encontra a distancia entre o ponteiro e o inicio da pagina
         unsigned int distancia = (unsigned long)ptr % getpagesize();
         unsigned long int enderecoFisico = enderecoQuadro << PAGE_SHIFT + distancia;
         printf("Endereco Fisico do Ponteiro: %lu\n", enderecoFisico);
      } else {
         printf("Filho: \n");
         printf("Endereco Logico: %p\n", ptr);
         unsigned long int enderecoQuadro = getNumeroPageFrame(ptr);
         printf("Endereco do quadro: %lu\n", enderecoQuadro);
         // encontra a distancia entre o ponteiro e o inicio da pagina
         unsigned int distancia = (unsigned long)ptr % getpagesize();
         unsigned long int enderecoFisico = enderecoQuadro << PAGE_SHIFT + distancia;
         printf("Endereco Fisico do Ponteiro: %lu\n", enderecoFisico);
      }
   }

   exit (0) ;
}
