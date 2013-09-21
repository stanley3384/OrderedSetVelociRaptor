

! 
!
!Build with make and makefile.
!
!Create a subroutine to interop with C and match up with Alan Genz's critical values Fortran code.
!
!Permission granted by Alan Genz to be able to use his Fortran code in this project. Very thankful
!for this. Some very difficult math and programming.
!
!See the references file for more information.
!
!

SUBROUTINE MVDISTDUN(N,COVRNC1,NU,M,LOWER,CONSTR,UPPER,INFIN,DELTA,MAXPTS,&
                     ABSEPS,RELEPS,ERROR,VALUE,NEVALS,INFORM,ALPHA,TALPHA) BIND(C)
        !Line up arguments for C interop.
        USE ISO_C_BINDING, ONLY: c_double, c_int
        IMPLICIT NONE
        INTEGER(c_int) :: N 
        INTEGER(c_int) :: NU
        INTEGER(c_int) :: M
        REAL(c_double) :: COVRNC1(N*N), CONSTR(M*N)
        REAL(c_double) :: COV1(N,N)
        REAL(c_double) :: CNS1(M,N)
        REAL(c_double) :: LOWER(M)
        REAL(c_double) :: UPPER(M) 
        INTEGER(c_int) :: INFIN(M) 
        REAL(c_double) :: DELTA(M)
        INTEGER(c_int) :: MAXPTS
        REAL(c_double) :: ABSEPS
        REAL(c_double) :: RELEPS
        REAL(c_double) :: ERROR
        REAL(c_double) :: VALUE
        INTEGER(c_int) :: NEVALS 
        INTEGER(c_int) :: INFORM 
        REAL(c_double) :: ALPHA
        REAL(c_double) :: TALPHA
        INTEGER(c_int) :: I
        INTEGER(c_int) :: J
        INTEGER(c_int) :: K = 1
        
        LOWER=-1
        UPPER=1
        DELTA=0
        INFIN=2 

        CNS1=0

        DO I = 1, N
          CNS1(I,I) = 1
        END DO

        K = 1!Need to reset K for multiple calls
        DO I = 1, N
          DO J = 1, N
            COV1(I,J) = COVRNC1(K)
            !PRINT "(12X, ""Fortran Matrix "",F8.5, I3)", COV1(I,J), K
            K = K + 1
          END DO
        END DO
        
        !Check Matrix order
        DO I = 1, N
           !PRINT "(I3, 11F11.7)", I, COV1(I,1:I)
        END DO
        !PRINT "(12X, ""C2R1 "",F8.5)", COV1(1,2)

        CALL MVPRNT(N,COV1,NU,M,LOWER,CNS1,UPPER,INFIN,DELTA,MAXPTS,ABSEPS,"  Critical Values Example", 1,ALPHA,TALPHA)
         
    RETURN
    END

!MVPRNT from Alan Genz's critical value example. Add ALPHA and TALPHA so that the critical value is returned to C. 
 SUBROUTINE MVPRNT(N,COVRNC,NU,M,LOWER,CONSTR,UPPER,INFIN,DELTA,MAXPTS,ABSEPS,LABEL,IP,ALPHA,TALPHA)
      USE PRECISION_MODEL
      USE MVSTAT
      IMPLICIT NONE
      INTEGER,                         INTENT(IN) :: N, NU, M, MAXPTS, IP
      REAL(KIND=STND), DIMENSION(M),   INTENT(IN) :: LOWER, UPPER, DELTA
      REAL(KIND=STND), DIMENSION(N,N), INTENT(IN) :: COVRNC
      REAL(KIND=STND), DIMENSION(M,N), INTENT(IN) :: CONSTR
      INTEGER,         DIMENSION(M),   INTENT(IN) :: INFIN
      CHARACTER(LEN=*),                INTENT(IN) :: LABEL
      REAL(KIND=STND),                 INTENT(IN) :: ABSEPS
!
      INTEGER                                     :: I, INF, IVLS
      REAL(KIND=STND)                             :: ALPHA, TALPHA
      REAL(KIND=STND)                             :: ERROR, VALUE
!
      PRINT "(/10X,A)", LABEL
      PRINT "(""           Number of Dimensions is "",I5)", N
      PRINT "(""          Number of Constraints is "",I5)", M
      PRINT "(""      Number of Degrees of Freedom is "",I5)", NU
      PRINT "(""     Maximum # of Function Evaluations is "",I9)", MAXPTS
      IF ( IP .GT. 0 ) THEN
         PRINT "(""    Lower  Upper     Constraints "")"
         DO I = 1, M
            IF ( INFIN(I) < 0 ) THEN 
               PRINT "(I3, ""  -00    00   "", 7F7.3)",  CONSTR(I,1:M)
            ELSE IF ( INFIN(I) == 0 ) THEN 
               PRINT "(I3, ""  -00  "", 10F7.3)", I, UPPER(I), CONSTR(I,1:M)
            ELSE IF ( INFIN(I) == 1 ) THEN 
               PRINT "(I3,F7.3,""  00   "",9F7.3)", I, LOWER(I), CONSTR(I,1:M)
            ELSE 
               !Format runtime error. Real to int.
               !PRINT "(I3, 11F7.3)", I, LOWER(I), UPPER(I), CONSTR(I,1:M)
            ENDIF
         END DO
         PRINT "(""     Lower Left Covariance Matrix "")"
         DO I = 1, N
            PRINT "(I3, 100F5.2)", I, COVRNC(I,1:I)
         END DO
      END IF
      CALL MVDIST( N, COVRNC, NU, M, LOWER, CONSTR, UPPER, INFIN, DELTA,&
                   MAXPTS, ABSEPS, 0E0_STND, ERROR, VALUE, IVLS, INF )
      PRINT "(5X, ""Value(Error): "",F11.8,""("",F11.8,"")"")", VALUE, ERROR
      PRINT "(5X,""Evaluations(Inform): "",I8,""("",I2,"")"")", IVLS, INF
!Set alpha
      !ALPHA = 1.0E-1_STND
      !DO I = 1, 2
         CALL MVCRIT( N, COVRNC, NU, M,LOWER,CONSTR,UPPER,INFIN,  &
                      ALPHA, 100*MAXPTS, ABSEPS, ERROR, TALPHA, IVLS, INF ) 
         PRINT "("" Alpha, T_alpha, Work, Inform: "", F6.2, F11.6, I10, I5)", &
                    ALPHA, TALPHA, IVLS, INF
         !ALPHA = ALPHA/2
      !END DO
   END SUBROUTINE MVPRNT



