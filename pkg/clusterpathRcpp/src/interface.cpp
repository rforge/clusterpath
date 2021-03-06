#include <Rcpp.h>
#include "l1.h"
#include "l2.h"
 
using namespace Rcpp ;

SEXP tree2list(Cluster *c){
  int nrow=c->total;
  //printf("%d total\n",nrow);
  NumericVector alpha(nrow),lambda(nrow);
  IntegerVector i(nrow);
  int row=0;
  add_results(c,&alpha[0],&lambda[0],&i[0],&row);
  return DataFrame::create(Named("alpha",alpha),
			   Named("lambda",lambda),
			   Named("i",i));
}

// we process each dimension individually using this function
RcppExport SEXP join_clusters_convert(SEXP xR){
  NumericVector x(xR);
  Cluster *tree=make_clusters_l1(&x[0],x.length());
  SEXP L=tree2list(tree);
  delete_tree(tree);
  return L;
}

//construct a list that will be used in R to return a data.frame
SEXP res2list(Results *r){
  unsigned int nrow=r->iterations.size() * r->n,i,k,row=0;
  NumericMatrix alpha(nrow,r->p);
  IntegerVector ivec(nrow);
  NumericVector lambda(nrow),grad(nrow);
  std::list<Iteration*>::iterator l;
  //r->print();
  for(l=r->iterations.begin();l!=r->iterations.end();l++){
    for(i=0;i<r->n;i++){
      for(k=0;k<r->p;k++){
	//printf("%d %d %d %d %d\n",nrow,k,row,i,r->n);
	alpha[nrow*k+row] = (*l)->alpha[i+k*r->n];
      }
      lambda[row] = (*l)->lambda;
      grad[row] = (*l)->grad;
      ivec[row] = i;
      row++;
    }
  }
  return DataFrame::create(Named("alpha",alpha),
			   Named("i",ivec),
			   Named("lambda",lambda),
			   Named("grad",grad));
}

RcppExport SEXP calcW_convert(SEXP x_R,SEXP gamma){
  NumericMatrix x(x_R);
  unsigned int i, n = x.nrow();
  NumericVector w_R(n*(n-1)/2);
  SymNoDiag W(n);
  W.calcW(&x[0],x.ncol(),NumericVector(gamma)[0]);
  for(i=0;i<w_R.length();i++){
    w_R[i]=W.data[i];
  }
  return w_R;
}

RcppExport SEXP join_clusters2_restart_convert
(SEXP x_R,SEXP w_R,
 SEXP lambda,SEXP join_thresh,SEXP opt_thresh,SEXP lambda_factor,SEXP smooth,
 SEXP maxit,SEXP linesearch_freq,SEXP linesearch_points,SEXP check_splits,
 SEXP target_cluster,
 SEXP verbose
 ){
  NumericMatrix x(x_R);
  unsigned int i, N=x.nrow();
  SymNoDiag W(N);
  NumericVector wvec(w_R);
  for(i=0;i<W.length;i++){
    W.data[i] = wvec[i];
  }
  Results *r = join_clusters2_restart
    (&x[0],
     &W,
     x.ncol(),
     NumericVector(lambda)[0],
     NumericVector(join_thresh)[0],
     NumericVector(opt_thresh)[0],
     NumericVector(lambda_factor)[0],
     NumericVector(smooth)[0],
     IntegerVector(maxit)[0],
     IntegerVector(linesearch_freq)[0],
     IntegerVector(linesearch_points)[0],
     IntegerVector(check_splits)[0],
     IntegerVector(target_cluster)[0],
     IntegerVector(verbose)[0]);
  SEXP L = res2list(r);
  delete r;
  return L;
}

