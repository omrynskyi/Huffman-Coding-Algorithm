#include "bitwriter.h"
#include "node.h"
#include "pq.h"

#include <getopt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Code {
    uint64_t code;
    uint8_t code_length;
} Code;

uint64_t fill_histogram(Buffer *inbuf, double *histogram) {
    for (int i = 0; i < 256; i++) {
        histogram[i] = 0;
    }

    uint64_t filesize = 0;
    uint8_t byte;

    while (read_uint8(inbuf, &byte)) {
        ++histogram[byte];
        ++filesize;
    }

    // Apply the hack to ensure at least two non-zero values in the histogram
    ++histogram[0x00];
    ++histogram[0xff];

    return filesize;
}

Node *create_tree(double *histogram, uint16_t *num_leaves) {
    PriorityQueue *pq = pq_create();
    for (int i = 0; i < 256; i++) {
        if (histogram[i] > 0.00) {
            Node *node = node_create(i, histogram[i]);
            enqueue(pq, node);
            (*num_leaves)++;
        }
    }

    while (!pq_size_is_1(pq)) {
        Node *left, *right;
        dequeue(pq, &left);
        dequeue(pq, &right);

        Node *new_node = node_create(-1, left->weight + right->weight);
        new_node->left = left;
        new_node->right = right;

        enqueue(pq, new_node);
    }

    Node *tree;
    dequeue(pq, &tree);

    pq_free(&pq);

    return tree;
}

void fill_code_table(Code *code_table, Node *node, uint64_t code, uint8_t code_length) {
    if (node->left != NULL && node->right != NULL) {
        // Internal node
        fill_code_table(code_table, node->left, code, code_length + 1);
        code |= ((uint64_t) 1 << code_length);
        fill_code_table(code_table, node->right, code, code_length + 1);
    } else {
        // Leaf node: store the Huffman Code
        code_table[node->symbol].code = code;

        code_table[node->symbol].code_length = code_length;
    }
}

void huff_write_tree(BitWriter *outbuf, Node *node) {
    if (node->left != NULL && node->right != NULL) {
        // Internal node
        huff_write_tree(outbuf, node->left);
        huff_write_tree(outbuf, node->right);
        bit_write_bit(outbuf, 0); // Write a '0' bit
    } else {
        // Leaf node
        bit_write_bit(outbuf, 1); // Write a '1' bit
        bit_write_uint8(outbuf, node->symbol); // Write the symbol
    }
}

void huff_compress_file(BitWriter *outbuf, Buffer *inbuf, uint32_t filesize, uint16_t num_leaves,
    Node *code_tree, Code *code_table) {
    bit_write_uint8(outbuf, 'H'); // Magic number
    bit_write_uint8(outbuf, 'C'); // Magic number
    bit_write_uint32(outbuf, filesize); // File size
    bit_write_uint16(outbuf, num_leaves); // Number of leaves

    node_print_tree(code_tree, '<', 3);

    huff_write_tree(outbuf, code_tree); // Write the code tree

    uint8_t byte;
    while (read_uint8(inbuf, &byte)) {
        uint64_t code = code_table[byte].code;
        printf("%lu \n", code);
        uint8_t code_length = code_table[byte].code_length;
        for (int i = 0; i < code_length; i++) {
            bit_write_bit(outbuf, code & 1); // Write the rightmost bit of the code
            code >>= 1; // Prepare to write the next bit
        }
    }
    node_free(&code_tree);
}

void print_help(void) {
    printf("Usage: huffman -i input_file -o output_file\n");
    printf("Options:\n");
    printf("  -i : Sets the name of the input file\n");
    printf("  -o : Sets the name of the output file\n");
    printf("  -h : Prints this help message\n");
}

int main(int argc, char *argv[]) {
    char *input_file_name = NULL;
    char *output_file_name = NULL;

    opterr = 0;
    int option;
    while ((option = getopt(argc, argv, "i:o:h")) != -1) {
        switch (option) {
        case 'i': input_file_name = optarg; break;
        case 'o': output_file_name = optarg; break;
        case 'h': print_help(); return 0;
        default: print_help(); return 1;
        }
    }

    // Check if required options are provided
    if (input_file_name == NULL) {
        printf("huff:  -i option is required\n");
        return 1;
    }
    if (output_file_name == NULL) {
        printf("huff:  -o option is required\n");
        return 1;
    }

    Buffer *input_file = read_open(input_file_name);
    if (input_file == NULL) {
        printf("Failed to open input file\n");
        return 1;
    }
    double *histogram = (double *) malloc(256 * sizeof(double));

    uint64_t filesize = fill_histogram(input_file, histogram);

    uint16_t num_leaves = 0;
    Node *code_tree = create_tree(histogram, &num_leaves);

    Code *code_table = (Code *) malloc(256 * sizeof(Code));
    fill_code_table(code_table, code_tree, 0, 0);

    read_close(&input_file);
    Buffer *read = read_open(input_file_name);
    if (read == NULL) {
        fprintf(stderr, "failed to open");
    }
    BitWriter *output_file = bit_write_open(output_file_name);
    if (output_file == NULL) {
        fprintf(stderr, "failed to open");
    }
    huff_compress_file(output_file, read, filesize, num_leaves, code_tree, code_table);

    bit_write_close(&output_file);
    free(histogram);
    free(code_table);
    read_close(&read);
    return 0;
}
