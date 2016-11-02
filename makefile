

OBJECTS=mvdist.o FortranInterop.o HotellingsT2.o VelociRaptorMath.o VelociRaptorUI_Validation.o VelociRaptorPrinting.o VelociRaptorPermutations.o VelociRaptorHtmlTable.o
PROGNAME=VelociRaptor
CFLAGS=`pkg-config --cflags gtk+-3.0` -std=c99 -Wall -Wno-deprecated-declarations -D_GNU_SOURCE=1 -O2 -fopenmp
LINKFLAGS=-lm -lapophenia -lgsl -lgslcblas -lsqlite3 -lgfortran `pkg-config --libs gtk+-3.0`
COMPILE1=gfortran -c $< -o $@ 
COMPILE2=gcc `pkg-config --cflags gtk+-3.0` -std=c99 -Wall -D_GNU_SOURCE=1 -O2 -c HotellingsT2.c -o HotellingsT2.o -lgsl -lgslcblas -lsqlite3 `pkg-config --libs gtk+-3.0`
COMPILE3=gcc `pkg-config --cflags gtk+-3.0` -std=c99 -Wall -D_GNU_SOURCE=1 -O2 -c VelociRaptorMath.c -o VelociRaptorMath.o -lgsl -lgslcblas `pkg-config --libs gtk+-3.0`
COMPILE4=gcc `pkg-config --cflags gtk+-3.0` -std=c99 -Wall -D_GNU_SOURCE=1 -O2 -c VelociRaptorUI_Validation.c -o VelociRaptorUI_Validation.o -lapophenia -lgsl -lgslcblas -lsqlite3 `pkg-config --libs gtk+-3.0`
COMPILE5=gcc `pkg-config --cflags gtk+-3.0` -std=c99 -Wall -Wno-deprecated-declarations -D_GNU_SOURCE=1 -O2 -c VelociRaptorPrinting.c -o VelociRaptorPrinting.o `pkg-config --libs gtk+-3.0`
COMPILE6=gcc `pkg-config --cflags gtk+-3.0` -std=c99 -fopenmp -Wall -D_GNU_SOURCE=1 -O2 -c VelociRaptorPermutations.c -o VelociRaptorPermutations.o `pkg-config --libs gtk+-3.0`
COMPILE7=gcc `pkg-config --cflags gtk+-3.0` -std=c99 -Wall -D_GNU_SOURCE=1 -O2 -c VelociRaptorHtmlTable.c -o VelociRaptorHtmlTable.o `pkg-config --libs gtk+-3.0`
COMPILE8=gcc -Wall -O2 -Wno-deprecated-declarations OSVreport_generator.c -o OSVreport_generator -I/usr/include/json-glib-1.0 `pkg-config --cflags --libs gtk+-3.0` -ljson-glib-1.0 -lsqlite3 -lm
COMPILE9=gcc -Wall -O2 -Wno-deprecated-declarations simple_sqlite_viewer.c -o simple_sqlite_viewer `pkg-config --cflags --libs gtk+-3.0` -lsqlite3

all: executable1 executable2 executable3

executable1:$(OBJECTS)
	@echo [CC] VelociRaptor
	@gcc $(CFLAGS) $(OBJECTS) VelociRaptorUI.c $(LINKFLAGS) -o $(PROGNAME)

executable2:OSVreport_generator.c
	@echo [CC] OSVreport_generator
	@$(COMPILE8)

executable3:simple_sqlite_viewer.c
	@echo [CC] simple_sqlite_viewer
	@$(COMPILE9)
	
mvdist.o: mvdist.f90
	@echo [FC] $@
	@$(COMPILE1)

FortranInterop.o: FortranInterop.f90
	@echo [FC] $@
	@$(COMPILE1) 

HotellingsT2.o: HotellingsT2.c
	@echo [CC] $@
	@$(COMPILE2)

VelociRaptorMath.o: VelociRaptorMath.c
	@echo [CC] $@
	@$(COMPILE3)

VelociRaptorUI_Validation.o: VelociRaptorUI_Validation.c
	@echo [CC] $@
	@$(COMPILE4)

VelociRaptorPrinting.o: VelociRaptorPrinting.c
	@echo [CC] $@
	@$(COMPILE5)

VelociRaptorPermutations.o: VelociRaptorPermutations.c
	@echo [CC] $@
	@$(COMPILE6)

VelociRaptorHtmlTable.o: VelociRaptorHtmlTable.c
	@echo [CC] $@
	@$(COMPILE7)

clean:
	rm VelociRaptor OSVreport_generator simple_sqlite_viewer mvdist.o FortranInterop.o mvstat.mod precision_model.mod HotellingsT2.o VelociRaptorMath.o VelociRaptorUI_Validation.o VelociRaptorPrinting.o VelociRaptorPermutations.o VelociRaptorHtmlTable.o
