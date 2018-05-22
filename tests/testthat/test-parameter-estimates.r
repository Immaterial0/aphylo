context("Parameter estimation")

rm(list = ls())
set.seed(154)
n <- 500
P <- 1
psi <- c(.01, .01)
mu  <- c(.02, .05)
Pi  <- .2

dat <- sim_annotated_tree(n, P=P, psi = psi, mu = mu, Pi = Pi)

# Estimation via L-BFGS-B
ans0 <- aphylo_mle(dat ~ mu + psi + eta + Pi, params = c(.05, .05, .05, .05, .9, .9, .5))

# Methods ----------------------------------------------------------------------
test_that("Methods", {
  
  # Printing
  expect_output(suppressWarnings(print(ans0)), "ESTIMATION OF")
  
  # Plotting
  expect_silent(plot(ans0))
  expect_silent(plot_LogLike(ans0))
  
  # Extracting coef and others
  expect_equal(coef(ans0), ans0$par)
  expect_equal(vcov(ans0), ans0$varcovar)
  
  
})

# ------------------------------------------------------------------------------
test_that("MCMC", {
  ans1 <- suppressWarnings(
    aphylo_mcmc(dat ~ mu + psi + eta + Pi, params = ans0$par,
                control = list(nbatch = 1e4, burnin=500, thin=20))
  )
  
  # Checking expectations
  expect_equal(ans0$par, ans1$par, tolerance = 0.2)
  
  # Plotting
  expect_silent(plot(ans1))
  expect_silent(plot_LogLike(ans1))
  
  # Extracting coef and others
  expect_equal(coef(ans1), ans1$par)
  expect_equal(vcov(ans1), ans1$varcovar)
})

test_that("MCMC: in a degenerate case all parameters goes to the prior", {
  
  
  set.seed(1)
  dat <- suppressWarnings(sim_annotated_tree(10, Pi=0, mu=c(0, 0), psi=c(0,0)))
  dat$tip.annotation[] <- 9L
  
  ans1 <- suppressWarnings(
    aphylo_mcmc(dat ~ mu + psi + eta(0,1) + Pi,
                params = c(rep(2/12, 4), .5, .5,2/12),
                priors = function(x) dbeta(x, 2, 10),
                control = list(nbatch = 2e4), check.informative = FALSE)
    )
  
  ans2 <- suppressWarnings(
    aphylo_mcmc(
      dat ~ mu + psi + eta(0,1) + Pi,
      params = c(rep(2/22, 4), .5,.5,2/22), 
      priors = function(x) dbeta(x, 2, 20),
      control = list(nbatch = 2e4), check.informative = FALSE)
  )
  
  
  # Should converge to the prior
  expect_equal(unname(coef(ans1))[-c(5:6)], rep(2/12, 5), tol=.025)
  expect_equal(unname(coef(ans2))[-c(5:6)], rep(2/22, 5), tol=.025)
  
})
