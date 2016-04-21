#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#define USHRT_MAX 65535

bool m_bFirstFrameComing = false;
unsigned int m_ulInsertMapSeq = 0;
unsigned int m_ulPreRecvFrameTimestamp = 0;
unsigned short m_usReceivedMaxSeq = 0;
void ComputInsert(unsigned short rtpSequence ,unsigned int rtpTimestamp, unsigned int rtpSSRC)
{
    // compute insert sequence here
    int twoFrameSeDiff = 0;
    
    if(rtpSequence == 230)
    {
        int i;
        i = 0;
    }
    
    short diff =  (short)rtpSequence - (short)m_usReceivedMaxSeq;
    if (m_bFirstFrameComing == false) // first coming
    {
        m_ulInsertMapSeq = rtpSequence;
        m_bFirstFrameComing = true;
        m_usReceivedMaxSeq = rtpSequence;
        m_ulPreRecvFrameTimestamp = rtpTimestamp;
    }
    else
    {
        if( rtpTimestamp >= m_ulPreRecvFrameTimestamp )
        {
            // LSLR
            if( rtpSequence < (unsigned short)m_ulInsertMapSeq )
            {
                twoFrameSeDiff = USHRT_MAX - (unsigned short)m_ulInsertMapSeq + rtpSequence + 1;
            }
            else
            {
                twoFrameSeDiff = rtpSequence - (unsigned short)m_ulInsertMapSeq;
            }
        }
        else
        {
            // LSFR
            if( rtpSequence < (unsigned short)m_ulInsertMapSeq )
            {
                twoFrameSeDiff = rtpSequence - (unsigned short)m_ulInsertMapSeq;
            }
            else
            {
                if (diff < 0) // disorder
                {
                    twoFrameSeDiff = (unsigned short)(USHRT_MAX - rtpSequence + (unsigned short)m_ulInsertMapSeq + 1);
                    twoFrameSeDiff = -twoFrameSeDiff;
                    
                }
                else // something error in sending side, timestamp drift/reset etc.
                {
                    twoFrameSeDiff = rtpSequence - (unsigned short)m_ulInsertMapSeq;
                }
              }
        }
        
        
        if (diff >= 1) {
            
            // for example, receiving 1, 2, 3, 4, it total received 4 packets.
            
            // for example, receiving 1 3 2 4, total received
            // 3 - 1 + 4 -3  = 3 and adding received first packet, so it should be 4.
            
            // for example, receiving  65533, 65535, 0, 65534, 1, total received 2 + 1 + 1 =4
            // and adding first packet, so total should be 5.
            
            
            // for example, receiving  65533, 65535 , 65534, 1, total received 2 + 2  =4
            // and adding first packet, so total should be 5.
            
            m_usReceivedMaxSeq = rtpSequence;
            
        }
        //////////
        
        m_ulInsertMapSeq = m_ulInsertMapSeq + twoFrameSeDiff;
        m_ulPreRecvFrameTimestamp = rtpTimestamp;
      }
}

int main(int argc, char* argv[])
{
    if (argc < 3) {
        printf("Please run as exe input.csv output.csv\n");
        return 0;
    }
    
    FILE* fSourceFile = NULL;
    FILE* fDestFile = NULL;
    
    fSourceFile = fopen(argv[1], "rb");
    fDestFile = fopen(argv[2], "w");
    
    if (!fSourceFile || !fDestFile) {
        printf("Can't open source or destination file!\n");
        return 0;
    }
  
    fprintf(fDestFile, "seq, oldinsert, newinsert\n");
    while (!feof(fSourceFile)) {
        unsigned int seq = 0;
        unsigned int   oldInsertSeq = 0;
        unsigned int    timestamp = 0;
        fscanf(fSourceFile, "%u,%u,%u",&seq, &oldInsertSeq, &timestamp);
        ComputInsert(seq, timestamp, 1111);
        char writedata[250];
        sprintf(writedata, "%u,%u,%u\n", seq, oldInsertSeq, m_ulInsertMapSeq);
        fwrite(writedata, sizeof(char), strlen(writedata), fDestFile);
//        fprintf(fDestFile, "%u,%u,%u\n", seq, oldInsertSeq, m_ulInsertMapSeq);

    }
    
    fclose(fSourceFile);
    fclose(fDestFile);
    return 0;
}