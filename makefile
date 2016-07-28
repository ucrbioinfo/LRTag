all: LRTag.exe

LRTag.exe: main.o  sgl_ppl_loci_correlation.o SCP.o thread_control_office.o MDS_solver.o LR_SCP.o reaper_emulator.o LD_select_emulator.o multi_pop_select.o
	g++ -o LRTag.exe -lpthread main.o  sgl_ppl_loci_correlation.o SCP.o thread_control_office.o MDS_solver.o LR_SCP.o reaper_emulator.o LD_select_emulator.o multi_pop_select.o

main.o : main.cpp
	g++ -o main.o -c -g main.cpp
sgl_ppl_loci_correlation.o : sgl_ppl_loci_correlation.cpp sgl_ppl_loci_correlation.h
	g++ -o sgl_ppl_loci_correlation.o -c -g sgl_ppl_loci_correlation.cpp 
SCP.o : SCP.cpp SCP.h
	g++ -o SCP.o -c -g SCP.cpp 
thread_control_office.o : thread_control_office.cpp thread_control_office.h
	g++ -o thread_control_office.o -c -g thread_control_office.cpp 
MDS_solver.o : MDS_solver.cpp MDS_solver.h constants.h
	g++ -o MDS_solver.o -c -g MDS_solver.cpp 
LR_SCP.o : LR_SCP.cpp LR_SCP.h constants.h
	g++ -o LR_SCP.o -c -g LR_SCP.cpp 
reaper_emulator.o : reaper_emulator.cpp reaper_emulator.h constants.h
	g++ -o reaper_emulator.o -c -g reaper_emulator.cpp 
LD_select_emulator.o : LD_select_emulator.cpp LD_select_emulator.h constants.h
	g++ -o LD_select_emulator.o -c -g LD_select_emulator.cpp 
multi_pop_select.o : multi_pop_select.cpp multi_pop_select.h constants.h
	g++ -o multi_pop_select.o -c -g multi_pop_select.cpp 


clean: 
	rm -rf *.o; rm -rf *.exe; rm -rf *~; rm -rf *.out
