#
#     Combine a control set with a test set in the data format that multtest uses.
# Simple set-up. The control vector is matched to each test set and put in a matrix
# for the multtest minP function.
#     For testing with data in the VelociRaptorData.db or any SQLite database.
# Change the SQL and mask for running different test sets. It is setup to use 1000
# records with the control set the first set of 10 and 99 test sets of 10.
#     Still working on getting some of this functionality in the application.
# 
# Run with;
#     Rscript Test_SQL_minP.R
#

library(multtest, verbose = FALSE)
library(RSQLite)
Driver<-dbDriver("SQLite")
Connection<-dbConnect(Driver,"VelociRaptorData.db")
Results1<-dbSendQuery(Connection, "SELECT Percent FROM Data WHERE KeyID<11;")
Control<-fetch(Results1,10)
Control1<-c(as.numeric(unlist(Control)))
dbClearResult(Results1)
Results2<-dbSendQuery(Connection, "SELECT Percent FROM Data WHERE KeyID>10;")
Test<-fetch(Results2, 990)
Test1<-c(as.numeric(unlist(Test)))
dbClearResult(Results2)
dbDisconnect(Connection) 
mask<-c(0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1)
#Length of a control set and a test set are equal.
ControlLength<-length(Control1)
Rows<-(length(Test1)/length(Control1))
Columns<-2*length(Control1)
Size<-Rows*Columns
print(Rows)
print(Columns)
print(Size)
Test2<-1:Size
counter<-1
for(i in 1:Rows)
   {
     for(j in 1:ControlLength)
        {
          Test2[counter]<-Control1[j]
          counter<-counter+1
        }
     for(k in 1:ControlLength)
        {
          Test2[counter]<-Test1[((i-1)*(ControlLength)+k)]
          counter<-counter+1
        }
   }

mTest1<-matrix(Test2, nrow = Rows, ncol = Columns, byrow = TRUE, dimnames = NULL)
#mTest1[1:Rows, 1:Columns]
minP <- mt.minP(mTest1, mask, test="t", side = "abs", B=20000)
cat("\n")
minP



