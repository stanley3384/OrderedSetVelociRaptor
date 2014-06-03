

OBJECTS=mvdist.o FortranInterop.o HotellingsT2.o VelociRaptorMath.o VelociRaptorUI_Validation.o VelociRaptorPrinting.o VelociRaptorPermutations.o VelociRaptorHtmlTable.o
PROGNAME=VelociRaptor
CFLAGS=`pkg-config --cflags gtk+-3.0` -std=c99 -Wall -D_GNU_SOURCE=1 -O2 -fopenmp
LINKFLAGS=-lm -lapophenia -lgsl -lgslcblas -lsqlite3 -lgfortran `pkg-config --libs gtk+-3.0`
COMPILE1=gfortran -c $< -o $@ 
COMPILE2=gcc `pkg-config --cflags gtk+-3.0` -std=c99 -Wall -D_GNU_SOURCE=1 -O2 -c HotellingsT2.c -o HotellingsT2.o -lgsl -lgslcblas -lsqlite3 `pkg-config --libs gtk+-3.0`
COMPILE3=gcc `pkg-config --cflags gtk+-3.0` -std=c99 -Wall -D_GNU_SOURCE=1 -O2 -c VelociRaptorMath.c -o VelociRaptorMath.o -lgsl -lgslcblas `pkg-config --libs gtk+-3.0`
COMPILE4=gcc `pkg-config --cflags gtk+-3.0` -std=c99 -Wall -D_GNU_SOURCE=1 -O2 -c VelociRaptorUI_Validation.c -o VelociRaptorUI_Validation.o -lapophenia -lgsl -lgslcblas -lsqlite3 `pkg-config --libs gtk+-3.0`
COMPILE5=gcc `pkg-config --cflags gtk+-3.0` -std=c99 -Wall -D_GNU_SOURCE=1 -O2 -c VelociRaptorPrinting.c -o VelociRaptorPrinting.o `pkg-config --libs gtk+-3.0`
COMPILE6=gcc `pkg-config --cflags gtk+-3.0` -std=c99 -fopenmp -Wall -D_GNU_SOURCE=1 -O2 -c VelociRaptorPermutations.c -o VelociRaptorPermutations.o `pkg-config --libs gtk+-3.0`
COMPILE7=gcc `pkg-config --cflags gtk+-3.0` -std=c99 -Wall -D_GNU_SOURCE=1 -O2 -c VelociRaptorHtmlTable.c -o VelociRaptorHtmlTable.o `pkg-config --libs gtk+-3.0`

executable:$(OBJECTS)
	gcc $(CFLAGS) $(OBJECTS) VelociRaptorUI.c $(LINKFLAGS) -o $(PROGNAME)

mvdist.o: mvdist.f90
	$(COMPILE1)

FortranInterop.o: FortranInterop.f90
	$(COMPILE1) 

HotellingsT2.o: HotellingsT2.c
	$(COMPILE2)

VelociRaptorMath.o: VelociRaptorMath.c
	$(COMPILE3)

VelociRaptorUI_Validation.o: VelociRaptorUI_Validation.c
	$(COMPILE4)

VelociRaptorPrinting.o: VelociRaptorPrinting.c
	$(COMPILE5)

VelociRaptorPermutations.o: VelociRaptorPermutations.c
	$(COMPILE6)

VelociRaptorHtmlTable.o: VelociRaptorHtmlTable.c
	$(COMPILE7)

run: executable
	./$(PROGNAME)

clean:
	rm VelociRaptor mvdist.o FortranInterop.o mvstat.mod precision_model.mod HotellingsT2.o VelociRaptorMath.o VelociRaptorUI_Validation.o VelociRaptorPrinting.o VelociRaptorPermutations.o VelociRaptorHtmlTable.o
