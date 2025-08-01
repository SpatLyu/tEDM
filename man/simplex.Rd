% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/simplex.R
\name{simplex}
\alias{simplex}
\alias{simplex,data.frame-method}
\alias{simplex,list-method}
\title{simplex forecast}
\usage{
\S4method{simplex}{data.frame}(
  data,
  column,
  target,
  lib = NULL,
  pred = NULL,
  E = 2:10,
  tau = 0,
  k = E + 1,
  threads = length(E)
)

\S4method{simplex}{list}(
  data,
  column,
  target,
  lib = NULL,
  pred = NULL,
  E = 2:10,
  tau = 0,
  k = E + 1,
  threads = length(E)
)
}
\arguments{
\item{data}{observation data.}

\item{column}{name of library variable.}

\item{target}{name of target variable.}

\item{lib}{(optional) libraries indices.}

\item{pred}{(optional) predictions indices.}

\item{E}{(optional) embedding dimensions.}

\item{tau}{(optional) step of time lags.}

\item{k}{(optional) number of nearest neighbors used in prediction.}

\item{threads}{(optional) number of threads to use.}
}
\value{
A list
\describe{
\item{\code{xmap}}{forecast performance}
\item{\code{varname}}{name of target variable}
\item{\code{method}}{method of cross mapping}
\item{\code{tau}}{step of time lag}
}
}
\description{
simplex forecast
}
\examples{
sim = logistic_map(x = 0.4,y = 0.4,step = 45,beta_xy = 0.5,beta_yx = 0)
simplex(sim,"x","y",E = 4:10,k = 7,threads = 1)

}
\references{
Sugihara G. and May R. 1990. Nonlinear forecasting as a way of distinguishing chaos from measurement error in time series. Nature, 344:734-741.
}
