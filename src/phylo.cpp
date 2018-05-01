// [[Rcpp::depends(RcppArmadillo)]]
#include <RcppArmadillo.h>
#include "misc.h"
using namespace Rcpp;

// Root node probabilities
// 
// @template parameters
// @templateVar Pi 1
// @templateVar S 1
// 
// Generates a vector of length 2^P with the root node probabilities
// per state
// @export
// [[Rcpp::export]]
arma::vec root_node_prob(
    double Pi,
    const arma::imat & S
) {
  // Obtaining relevant constants
  int P       = S.n_cols;
  int nstates = S.n_rows;
  
  arma::vec ans(nstates);
  ans.ones();
  
  for (int s=0; s<nstates; s++)
    for (int p=0; p<P; p++)
      ans.at(s) *= (S.at(s,p) == 0)? (1.0 - Pi) : Pi;
  
  return ans;
}

//' State probabilities
//' 
//' Compute the state probabilities for each node in the tree using the peeling
//' algorithm. This function is the horse-power of the function \code{\link{LogLike}}
//' so it is not intended to be used directly.
//' 
//' @template parameters
//' @templateVar annotations 1
//' @templateVar mu 1
//' @templateVar psi 1
//' @templateVar eta 1
//' @templateVar S 1
//' @templateVar offspring 1
//' @param Pr A matrix.
//' 
//' @return A numeric matrix of size \eqn{n\times 2^P}{n * 2^P} with state
//' probabilities for each node.
//' @noRd
// [[Rcpp::export(rng = false)]]
arma::mat probabilities(
    const arma::imat & annotations,
    const arma::ivec & pseq,
    const arma::vec  & psi,
    const arma::vec  & mu,
    const arma::vec  & eta,
    const arma::imat & S,
    const List       & offspring
) {
  
  // Obtaining relevant constants
  int P         = S.n_cols;
  int nstates   = S.n_rows;
  arma::mat M   = prob_mat(mu);
  arma::mat PSI = prob_mat(psi);
  arma::mat Pr(annotations.n_rows, nstates, arma::fill::ones);
  
  typedef arma::ivec::const_iterator iviter;
  typedef IntegerVector::const_iterator Riviter;
  
  for (iviter n = pseq.begin(); n != pseq.end(); ++n) {
    
    // Rprintf("Looping in n=%i\n", n);
    // Only for internal nodes
    if (! (bool) Rf_length(offspring.at(*n - 1u))) {
      
      for (int s=0; s<nstates; s++)
        for (int p=0; p<P; p++) {
          
          // Annotation bias probabilities:
          // eta[0]: Probability of annotating a 0.
          // eta[1]: Probability of annotating a 1.
          
          // If missing (no experimental data)
          if (annotations.at(*n - 1u, p) == 9) {
            Pr.at(*n - 1u, s) *= 
              (1.0 - eta.at(0u)) * PSI.at(S.at(s, p), 0u) + 
              (1.0 - eta.at(1u)) * PSI.at(S.at(s, p), 1u)
            ;
            continue;
          }
            
          Pr.at(*n - 1u, s) *= PSI.at(S.at(s, p), annotations.at(*n - 1u, p))*
            eta.at(annotations.at(*n - 1u, p));
          
        }
        continue;
      
    }
      
    // Obtaining list of offspring <- this can be improved (speed)
    // can create an std vector of size n
    IntegerVector O(offspring.at(*n - 1u));
    
    // Parent node states integration
    for (int s=0; s<nstates; s++) {
      
      // Loop through offspring
      double offspring_joint_likelihood = 1.0;
      
      for (Riviter o_n = O.begin(); o_n != O.end() ; ++o_n) {
        
        // Offspring states integration
        double offspring_likelihood = 0.0;
        for (int s_n=0; s_n<nstates; s_n++) {
          double s_n_sum = 1.0;
          
          // Loop through functions
          for (int p=0; p<P; p++) 
            s_n_sum *= M.at(S.at(s,p), S.at(s_n,p));
          
          // Multiplying by prob of offspring
          offspring_likelihood += ( s_n_sum * Pr.at(*o_n - 1u, s_n) );
          
        }
        
        // Multiplying with other offspring
        offspring_joint_likelihood *= offspring_likelihood;
        
      }
      
      // Adding state probability
      Pr.at(*n - 1u, s) = offspring_joint_likelihood;
      
    }
  }
  
  return Pr;
}

// Computes Log-likelihood
// [[Rcpp::export(name = ".LogLike", rng = false)]]
List LogLike(
    const arma::imat & annotations,
    const List       & offspring,
    const arma::ivec & pseq,
    const arma::vec  & psi,
    const arma::vec  & mu,
    const arma::vec  & eta,
    double Pi,
    bool verb_ans = false,
    bool check_dims = true
) {

  // Checking dimmensions
  if (check_dims) {
    
    bool dims_are_ok = true;
    
    // Data dims
    int n_annotations = annotations.n_rows;
    int n_offspring   = offspring.size();
    
    if (n_annotations != n_offspring) {
      warning("-annotations- and -offspring- have different lengths.");
      dims_are_ok = false;
    }
      
    // Parameters dims
    int n_psi = psi.size();
    int n_mu  = mu.size();
    
    if (n_psi != 2) {
      warning("-psi- must be a vector of size 2.");
      dims_are_ok = false;
    }
    
    if (n_mu != 2) {
      warning("-mu- must be a vector of size 2.");
      dims_are_ok = false;
    }
    
    
    // Return with error
    if (!dims_are_ok)
      stop("Check the size of the inputs.");
    
  }
  
  // Obtaining States, PSI, Gain/Loss probs, and root node probs
  arma::imat S  = states(annotations.n_cols);
  int nstates   = (int) S.n_rows;

  // Computing likelihood
  arma::mat Pr  = probabilities(annotations, pseq, psi, mu, eta, S, offspring);

  // We only use the root node
  double ll = 0.0;

  arma::vec PiP = root_node_prob(Pi, S);  
  for (int s = 0; s<nstates; s++)
    ll += PiP.at(s)*Pr.at(pseq.at(pseq.size() - 1u) - 1u, s);
  
  ll = std::log(ll);
  
  // return ll;
  if (verb_ans) {
    
    List ans = List::create(
      _["S"]   = S,
      _["Pr"]   = Pr,
      _["ll"]  = ll
    );
    ans.attr("class") = "phylo_LogLik";
    return(ans);
    
  } else {
    
    List ans = List::create(_["ll"] = ll);
    ans.attr("class") = "phylo_LogLik";
    return(ans);
    
  }
  
}

