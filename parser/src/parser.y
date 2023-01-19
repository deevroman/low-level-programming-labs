%require "3.7"
%code requires {
	#define YYERROR_VERBOSE 1
	void yyerror(const char *s);
	int yylex();
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#include <stdbool.h>
	#include <stdint.h>
	#include <ctype.h>
    #include "types.h"
}

%define parse.error verbose
%locations
%union {
	uint64_t num;
	char *string;
	double fnum;
	_Bool b;
	struct filter *filter_;
	struct new_field *new_field_;
	struct value value_;
  	struct field_key_value field_key_value_;
}
%token DB
%token FIND INSERT DELETE UPDATE
%token <string> STRING
%token SET OR AND
%token LT LET GT GET NE REGEX
%token OPBRACE CLBRACE
%token OPCBRACE CLCBRACE
%token OPSQBRACE CLSQBRACE
%token COLON DOLLAR COMMA QUOTE
%token FALSE TRUE
%token <num> INT_NUMBER
%token <fnum> DOUBLE_NUMBER
%type <num> bool comp 
%type <filter_> filter filters or_filters
%type <new_field_> new_vals
%type <value_> value_literal
%type <field_key_value_> new_val

%{
	static struct query q = {0};
	static size_t allocations_size = 0;

	#ifndef DEBUG
	static void *custom_malloc(size_t size){
		allocations_size += size;
		return malloc(size);
	}
	#else
	#DEFINE custom_malloc(size) malloc(size)
	#endif

	static void print_allocations_size(){
		printf("Allocations size: %zu bytes\n", allocations_size);
	}

	void yyerror(const char *s) {
		fprintf (stderr, "%s\n", s);
	}

%}

%%

mongosh: query {print_query(q); print_allocations_size(); YYACCEPT;};

query: 
      | DB INSERT OPBRACE INT_NUMBER COMMA QUOTE STRING QUOTE COMMA OPCBRACE new_vals CLCBRACE CLBRACE {
		q.command = CMD_INSERT;
		q.parent = $4;
		q.schema = $7;
		q.new_fields = $11;
	  }
	  | DB FIND OPBRACE QUOTE STRING QUOTE COMMA OPCBRACE filters CLCBRACE CLBRACE {
		q.command = CMD_FIND;
		q.schema = $5;
		q.cond = $9;
	  }
	  | DB UPDATE OPBRACE QUOTE STRING QUOTE COMMA OPCBRACE filters CLCBRACE COMMA OPCBRACE DOLLAR SET COLON OPCBRACE new_vals CLCBRACE CLCBRACE CLBRACE {
		q.command  = CMD_UPDATE;
		q.schema = $5;
		q.cond = $9;
		q.new_fields = $17;
	  }
	  | DB DELETE OPBRACE QUOTE STRING QUOTE COMMA OPCBRACE filters CLCBRACE CLBRACE {
		q.command = CMD_DELETE;
		q.cond = $9;
	  }
;

filters : filter {
		$$ = $1;
	}
	| filter COMMA filters {
		$$ = custom_malloc(sizeof(struct filter));
		$$->op = OP_AND;
		$$->left = $1;
		$$->right = $3;
		$$->level = $1->level;
	};

or_filters : filter {
		$$ = $1;
	}
	| filter COMMA or_filters {
		$$ = custom_malloc(sizeof(struct filter));
		$$->op = OP_OR;
		$$->left = $1;
		$$->right = $3;
		$$->level = $1->level;
	};


filter : STRING COLON value_literal {
			$$ = custom_malloc(sizeof(struct filter));
			$$->op = OP_KEY_VALUE;
			$$->key_value.key = $1;
			$$->key_value.value = $3;
			$$->level = 0;
	 	}
	| STRING COLON OPCBRACE DOLLAR comp COLON value_literal CLCBRACE {
			$$ = custom_malloc(sizeof(struct filter));
			$$->op = OP_COMP;
			$$->key_value.key = $1;
			$$->key_value.value = $7;
			$$->comp = $5;
			$$->level = 0;
		}
	| DOLLAR OR OPSQBRACE or_filters CLSQBRACE {
			$$ = $4;
			$$->op = OP_OR;
			$$->level = $4->level + 1;
		}
	| DOLLAR AND OPSQBRACE filters CLSQBRACE {
			$$ = $4;
			$$->op = OP_AND;
			$$->level = $4->level + 1;
		}
;

new_vals : new_val {
		$$ = custom_malloc(sizeof(struct new_field));
		$$->next = 0;
		$$->field = $1;
	}
	| new_val COMMA new_vals {
		$$ = custom_malloc(sizeof(struct new_field));
		$$->next = $3;
		$$->field = $1;
	}
;

new_val : STRING COLON value_literal {
			$$.key = $1;
			$$.value = $3;
	 	};

value_literal : QUOTE STRING QUOTE {
		$$.value_type = DB_STRING;
		$$.data.str_value = $2;
	}
	| INT_NUMBER {
		$$.value_type = DB_INT32; 
		$$.data.int_value = $1;
	}
	| DOUBLE_NUMBER {
		$$.value_type = DB_DOUBLE;
		$$.data.double_value = $1;
	}
	| bool {
		$$.value_type = DB_BOOL;
		$$.data.bool_value = $1;
	}
;

bool : TRUE {$$ = 1;}
       | FALSE {$$ = 0;}
;

comp : LT {$$ = OP_LT;}
       | LET {$$ = OP_LET;}
       | GT {$$ = OP_GT;}
       | GET {$$ = OP_GET;}
       | NE {$$ = OP_NE;}
	   | REGEX {$$ = OP_NE;}
;

%%
