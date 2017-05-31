rm(list = ls())

load("playground/simulations/data_and_functions.rda")

set.seed(1223)
ans_MCMC_right_prior <- vector("list", nsim)
for (i in 1:nsim) {
  
  # MCMC estimators
  ans_MCMC_right_prior[[i]] <- mcmc_lite(
    dat_obs[[i]], rep(.1, 5), thin=200,
    priors = function(params) dbeta(params, 2, 20)
  )
  
  # Printing on screen (and saving)
  if (!(i %% 10)) {
    message(sprintf("Simulation %04i/%04i (%6.2f %%) complete.", i, nsim, i/nsim*100))
    
    # Just in case we need to restart this... so we can continue where we were
    curseed_MCMC_right_prior <- .Random.seed
    
    # Storing all objects
    save(curseed_MCMC_right_prior, ans_MCMC_right_prior, i, file = "playground/simulations/mcmc_right_prior_estimates.rda")
  } else message(".", appendLF = FALSE)
  
}

# Terminating
message(sprintf("Simulation %04i/%04i (%6.2f %%) complete.", i, nsim, i/nsim*100))

curseed_MCMC_right_prior <- .Random.seed
save(curseed_MCMC_right_prior, ans_MCMC_right_prior, i, file = "playground/simulations/mcmc_right_prior_estimates.rda")
