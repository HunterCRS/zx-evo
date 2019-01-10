#pragma once

struct MonLabel final
{
    u8 *address;
    unsigned name_offs;
};

struct MonLabelsT final
{
   MonLabel *pairs;
   unsigned n_pairs;
   char *names;
   unsigned names_size;

   MonLabelsT();
   ~MonLabelsT();

   unsigned add_name(char *name);
   void clear(const u8 *start, unsigned size);
   void clear_ram();
   void sort() const;

   char *find(const u8 *address) const;
   void add(u8 *address, char *name);
   unsigned load(char *filename, u8 *base, unsigned size);


   char xas_errstr[80]{};
   u8 xaspage{};
   void find_xas();

   enum { max_alasm_ltables = 16 };
   char alasm_valid_char[0x100]{};
   unsigned alasm_found_tables{};
   unsigned alasm_offset[max_alasm_ltables]{};
   unsigned alasm_count[max_alasm_ltables]{};
   unsigned alasm_chain_len(const u8 *page, unsigned offset, unsigned &end);
   void find_alasm();

   void import_menu();
   void import_xas();
   void import_alasm(unsigned offset, char *caption);


   HANDLE h_new_user_labels{};
   char userfile[0x200]{};
   void stop_watching_labels();
   void start_watching_labels();
   void notify_user_labels();
   void import_file();

};

extern MonLabelsT mon_labels;

void load_labels(char *filename, u8 *base, unsigned size);
void mon_show_labels();
void init_labels(char* filename);

