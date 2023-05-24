/*
 * MIT License
 *
 * Copyright (c) 2023 University of Oregon, Kevin Huck
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "zerosum.h"
#include "mpi.h"
#include <signal.h>

namespace zerosum {

    inline size_t getBytesTransferred(int count, MPI_Datatype datatype) {
        int typesize = 0;
        PMPI_Type_size( datatype, &typesize );
        size_t bytes = (size_t)(typesize) * (size_t)(count);
        return bytes;
    }

    int translateRankToWorld(MPI_Comm comm, int rank) {
        if (rank == MPI_ANY_SOURCE) return rank; // we don't know the source
        if (rank == MPI_ROOT) return rank; // we don't know the source
        typedef std::map<int, int> rank_map;
        static struct comm_map : public std::map<MPI_Comm, rank_map> {
            virtual ~comm_map() = default;
        } comms;

        if (comm != MPI_COMM_WORLD) {
            // If the rank_map doesn't exist, it is created
            rank_map & comm_ranks = comms[comm];

            rank_map::iterator it = comm_ranks.find(rank);
            if (it != comm_ranks.end()) {
                return it->second;
            } else {
                int result;
                int worldrank;

                PMPI_Comm_compare(comm, MPI_COMM_WORLD, &result);
                if (result == MPI_CONGRUENT || result == MPI_IDENT) {
                    worldrank = rank;
                } else {
                    MPI_Group commGroup;
                    MPI_Group worldGroup;
                    int ranks[1];
                    int worldranks[1];

                    ranks[0] = rank;
                    PMPI_Comm_group(MPI_COMM_WORLD, &worldGroup);
                    PMPI_Comm_group(comm, &commGroup);
                    //printf("translating rank: %d\n", ranks[0]);
                    PMPI_Group_translate_ranks(commGroup, 1, ranks, worldGroup, worldranks);

                    worldrank = worldranks[0];
                }
                comm_ranks[rank] = worldrank;
                return worldrank;
            }
        }
        return rank;
    }

}

extern "C" {

    int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest,
        int tag, MPI_Comm comm){
        /* Get the byte count */
        size_t bytes = zerosum::getBytesTransferred(count, datatype);
        zerosum::ZeroSum::getInstance().recordSentBytes(
            zerosum::translateRankToWorld(comm, dest), bytes);
        return PMPI_Send(buf, count, datatype, dest, tag, comm);
    }
#define ZEROSUM_MPI_SEND_TEMPLATE(_symbol) \
void  _symbol( void * buf, MPI_Fint * count, MPI_Fint * datatype, MPI_Fint * dest, \
    MPI_Fint * tag, MPI_Fint * comm, MPI_Fint * ierr ) { \
    *ierr = MPI_Send( buf, *count, MPI_Type_f2c(*datatype), *dest, *tag, MPI_Comm_f2c(*comm) ); \
}
    ZEROSUM_MPI_SEND_TEMPLATE(mpi_send)
    ZEROSUM_MPI_SEND_TEMPLATE(mpi_send_)
    ZEROSUM_MPI_SEND_TEMPLATE(mpi_send__)
    ZEROSUM_MPI_SEND_TEMPLATE(MPI_SEND)
    ZEROSUM_MPI_SEND_TEMPLATE(MPI_SEND_)
    ZEROSUM_MPI_SEND_TEMPLATE(MPI_SEND__)

    int MPI_Bsend(const void *buf, int count, MPI_Datatype datatype,
                              int dest, int tag, MPI_Comm comm){
        /* Get the byte count */
        size_t bytes = zerosum::getBytesTransferred(count, datatype);
        zerosum::ZeroSum::getInstance().recordSentBytes(
            zerosum::translateRankToWorld(comm, dest), bytes);
        return PMPI_Bsend(buf, count, datatype, dest, tag, comm);
    }
#define ZEROSUM_MPI_BSEND_TEMPLATE(_symbol) \
void  _symbol( void * buf, MPI_Fint * count, MPI_Fint * datatype, MPI_Fint * dest, \
    MPI_Fint * tag, MPI_Fint * comm, MPI_Fint * ierr ) { \
    *ierr = MPI_Bsend( buf, *count, MPI_Type_f2c(*datatype), *dest, *tag, MPI_Comm_f2c(*comm) ); \
}
    ZEROSUM_MPI_BSEND_TEMPLATE(mpi_bsend)
    ZEROSUM_MPI_BSEND_TEMPLATE(mpi_bsend_)
    ZEROSUM_MPI_BSEND_TEMPLATE(mpi_bsend__)
    ZEROSUM_MPI_BSEND_TEMPLATE(MPI_BSEND)
    ZEROSUM_MPI_BSEND_TEMPLATE(MPI_BSEND_)
    ZEROSUM_MPI_BSEND_TEMPLATE(MPI_BSEND__)

    int MPI_Rsend(const void *ibuf, int count, MPI_Datatype datatype, int dest,
                             int tag, MPI_Comm comm){
        /* Get the byte count */
        size_t bytes = zerosum::getBytesTransferred(count, datatype);
        zerosum::ZeroSum::getInstance().recordSentBytes(
            zerosum::translateRankToWorld(comm, dest), bytes);
        return PMPI_Rsend(ibuf, count, datatype, dest, tag, comm);
    }
#define ZEROSUM_MPI_RSEND_TEMPLATE(_symbol) \
void  _symbol( void * ibuf, MPI_Fint * count, MPI_Fint * datatype, MPI_Fint * dest, \
    MPI_Fint * tag, MPI_Fint * comm, MPI_Fint * ierr ) { \
    *ierr = MPI_Rsend( ibuf, *count, MPI_Type_f2c(*datatype), *dest, *tag, MPI_Comm_f2c(*comm) ); \
}
    ZEROSUM_MPI_RSEND_TEMPLATE(mpi_rsend)
    ZEROSUM_MPI_RSEND_TEMPLATE(mpi_rsend_)
    ZEROSUM_MPI_RSEND_TEMPLATE(mpi_rsend__)
    ZEROSUM_MPI_RSEND_TEMPLATE(MPI_RSEND)
    ZEROSUM_MPI_RSEND_TEMPLATE(MPI_RSEND_)
    ZEROSUM_MPI_RSEND_TEMPLATE(MPI_RSEND__)

    int MPI_Ssend(const void *buf, int count, MPI_Datatype datatype, int dest,
                             int tag, MPI_Comm comm){
        /* Get the byte count */
        size_t bytes = zerosum::getBytesTransferred(count, datatype);
        zerosum::ZeroSum::getInstance().recordSentBytes(
            zerosum::translateRankToWorld(comm, dest), bytes);
        return PMPI_Ssend(buf, count, datatype, dest, tag, comm);
    }
#define ZEROSUM_MPI_SSEND_TEMPLATE(_symbol) \
void  _symbol( void * buf, MPI_Fint * count, MPI_Fint * datatype, MPI_Fint * dest, \
    MPI_Fint * tag, MPI_Fint * comm, MPI_Fint * ierr ) { \
    *ierr = MPI_Ssend( buf, *count, MPI_Type_f2c(*datatype), *dest, *tag, MPI_Comm_f2c(*comm) ); \
}
    ZEROSUM_MPI_SSEND_TEMPLATE(mpi_ssend)
    ZEROSUM_MPI_SSEND_TEMPLATE(mpi_ssend_)
    ZEROSUM_MPI_SSEND_TEMPLATE(mpi_ssend__)
    ZEROSUM_MPI_SSEND_TEMPLATE(MPI_SSEND)
    ZEROSUM_MPI_SSEND_TEMPLATE(MPI_SSEND_)
    ZEROSUM_MPI_SSEND_TEMPLATE(MPI_SSEND__)

    int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest,
            int tag, MPI_Comm comm, MPI_Request *request) {
        /* Get the byte count */
        size_t bytes = zerosum::getBytesTransferred(count, datatype);
        zerosum::ZeroSum::getInstance().recordSentBytes(
            zerosum::translateRankToWorld(comm, dest), bytes);
        return PMPI_Isend(buf, count, datatype, dest, tag, comm, request);
    }
#define ZEROSUM_MPI_ISEND_TEMPLATE(_symbol) \
void  _symbol( void * buf, MPI_Fint * count, MPI_Fint * datatype, MPI_Fint * dest, \
    MPI_Fint * tag, MPI_Fint * comm, MPI_Fint * request, MPI_Fint * ierr ) { \
    MPI_Request local_request; \
    *ierr = MPI_Isend( buf, *count, MPI_Type_f2c(*datatype), *dest, *tag, MPI_Comm_f2c(*comm), &local_request ); \
    *request = MPI_Request_c2f(local_request); \
}
    ZEROSUM_MPI_ISEND_TEMPLATE(mpi_isend)
    ZEROSUM_MPI_ISEND_TEMPLATE(mpi_isend_)
    ZEROSUM_MPI_ISEND_TEMPLATE(mpi_isend__)
    ZEROSUM_MPI_ISEND_TEMPLATE(MPI_ISEND)
    ZEROSUM_MPI_ISEND_TEMPLATE(MPI_ISEND_)
    ZEROSUM_MPI_ISEND_TEMPLATE(MPI_ISEND__)

    int MPI_Recv(void *buf, int count, MPI_Datatype datatype,
        int source, int tag, MPI_Comm comm, MPI_Status *status){
        /* Get the byte count */
        double bytes = zerosum::getBytesTransferred(count, datatype);
        zerosum::ZeroSum::getInstance().recordRecvBytes(
            zerosum::translateRankToWorld(comm, source), bytes);
        return PMPI_Recv(buf, count, datatype, source, tag, comm, status);
    }
#define ZEROSUM_MPI_RECV_TEMPLATE(_symbol) \
void  _symbol( void * buf, MPI_Fint * count, MPI_Fint * datatype, MPI_Fint * source, \
    MPI_Fint * tag, MPI_Fint * comm, MPI_Fint * status, MPI_Fint * ierr ) { \
    MPI_Status s; \
    *ierr = MPI_Recv( buf, *count, MPI_Type_f2c(*datatype), *source, *tag, MPI_Comm_f2c(*comm), &s ); \
    MPI_Status_c2f(&s, status); \
}
    ZEROSUM_MPI_RECV_TEMPLATE(mpi_recv)
    ZEROSUM_MPI_RECV_TEMPLATE(mpi_recv_)
    ZEROSUM_MPI_RECV_TEMPLATE(mpi_recv__)
    ZEROSUM_MPI_RECV_TEMPLATE(MPI_RECV)
    ZEROSUM_MPI_RECV_TEMPLATE(MPI_RECV_)
    ZEROSUM_MPI_RECV_TEMPLATE(MPI_RECV__)

    int MPI_Irecv(void *buf, int count, MPI_Datatype datatype,
        int source, int tag, MPI_Comm comm, MPI_Request *request) {
        /* Get the byte count */
        double bytes = zerosum::getBytesTransferred(count, datatype);
        zerosum::ZeroSum::getInstance().recordRecvBytes(
            zerosum::translateRankToWorld(comm, source), bytes);
        return PMPI_Irecv(buf, count, datatype, source, tag, comm, request);
    }
#define ZEROSUM_MPI_IRECV_TEMPLATE(_symbol) \
void  _symbol( void * buf, MPI_Fint * count, MPI_Fint * datatype, MPI_Fint * source, \
    MPI_Fint * tag, MPI_Fint * comm, MPI_Fint * request, MPI_Fint * ierr ) { \
    MPI_Request local_request; \
    *ierr = MPI_Irecv( buf, *count, MPI_Type_f2c(*datatype), *source, *tag, MPI_Comm_f2c(*comm), &local_request ); \
    *request = MPI_Request_c2f(local_request); \
}
    ZEROSUM_MPI_IRECV_TEMPLATE(mpi_irecv)
    ZEROSUM_MPI_IRECV_TEMPLATE(mpi_irecv_)
    ZEROSUM_MPI_IRECV_TEMPLATE(mpi_irecv__)
    ZEROSUM_MPI_IRECV_TEMPLATE(MPI_IRECV)
    ZEROSUM_MPI_IRECV_TEMPLATE(MPI_IRECV_)
    ZEROSUM_MPI_IRECV_TEMPLATE(MPI_IRECV__)

    int MPI_Sendrecv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
        int dest, int sendtag, void *recvbuf, int recvcount, MPI_Datatype recvtype,
        int source, int recvtag, MPI_Comm comm, MPI_Status * status) {
        /* Get the byte count */
        double sbytes = zerosum::getBytesTransferred(sendcount, sendtype);
        double rbytes = zerosum::getBytesTransferred(recvcount, recvtype);
        zerosum::ZeroSum::getInstance().recordSentBytes(
            zerosum::translateRankToWorld(comm, dest), sbytes);
        zerosum::ZeroSum::getInstance().recordRecvBytes(
            zerosum::translateRankToWorld(comm, source), rbytes);
        return PMPI_Sendrecv(sendbuf, sendcount, sendtype, dest, sendtag, recvbuf, recvcount, recvtype,
                 source, recvtag, comm, status);
    }
#define ZEROSUM_MPI_SENDRECV_TEMPLATE(_symbol) \
void _symbol(void * sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *dest, \
    MPI_Fint *sendtag, void * recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *source, \
    MPI_Fint *recvtag, MPI_Fint *comm, MPI_Fint * status, MPI_Fint *ierr) { \
    MPI_Status local_status; \
    *ierr = MPI_Sendrecv( sendbuf, *sendcount, MPI_Type_f2c(*sendtype), *dest, *sendtag, \
        recvbuf, *recvcount, MPI_Type_f2c(*recvtype), *source, *recvtag, MPI_Comm_f2c(*comm), &local_status ); \
    MPI_Status_c2f(&local_status, status); \
}
    ZEROSUM_MPI_SENDRECV_TEMPLATE(mpi_sendrecv)
    ZEROSUM_MPI_SENDRECV_TEMPLATE(mpi_sendrecv_)
    ZEROSUM_MPI_SENDRECV_TEMPLATE(mpi_sendrecv__)
    ZEROSUM_MPI_SENDRECV_TEMPLATE(MPI_SENDRECV)
    ZEROSUM_MPI_SENDRECV_TEMPLATE(MPI_SENDRECV_)
    ZEROSUM_MPI_SENDRECV_TEMPLATE(MPI_SENDRECV__)

} // extern "C"