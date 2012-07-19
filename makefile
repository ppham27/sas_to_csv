OBJDIR := bin

all: $(OBJDIR) bin/sas_to_csv

$(OBJDIR) : 
	mkdir -p bin

bin/sas_to_csv: $(OBJDIR)
	gcc -O3 src/sas_to_csv.c -o bin/sas_to_csv
