#ifdef batch

      PROGRAM main
*
*
      PARAMETER (NGBANK=500000, NHBOOK=5000)
      COMMON/GCBANK/Q(NGBANK)
      COMMON/PAWC  /H(NHBOOK)
*
      CALL GZEBRA( NGBANK)
      CALL HLIMIT(-NHBOOK)
*
* *** initialize HIGZ
      CALL HPLINT(0)      
*
* *** GEANT initialisation
      CALL UGINIT
*
* *** Start events processing
      CALL GRUN
*
* *** End of RUN
      CALL UGLAST
*
      STOP
      END
      
#else
      
      PROGRAM main
*
*     GEANT main program. To link with the MOTIF user interface
*     the routine GPAWPP(NWGEAN,NWPAW) should be called, whereas
*     the routine GPAW(NWGEAN,NWPAW) gives access to the basic
*     graphics version.
*
      PARAMETER (NWGEAN=3000000, NWPAW=1000000)
      COMMON/GCBANK/GEANT(NWGEAN)
      COMMON/PAWC  /PAW  (NWPAW)
*
*
      CALL GPAW (NWGEAN,NWPAW)
*
      END
*
      SUBROUTINE qnext
      END
*
      SUBROUTINE czopen
      END
*
      SUBROUTINE cztcp
      END
*
      SUBROUTINE czclos
      END
*
      SUBROUTINE czputa
      END
      
#endif      
