% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/ic.R
\name{ic}
\alias{ic}
\alias{ic,data.frame-method}
\title{intersection cardinality}
\usage{
\S4method{ic}{data.frame}(
  data,
  column,
  target,
  lib = NULL,
  pred = NULL,
  E = 2:10,
  tau = 0,
  k = E + 2,
  threads = length(pred),
  parallel.level = "low"
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

\item{parallel.level}{(optional) level of parallelism, \code{low} or \code{high}.}
}
\value{
A list
\describe{
\item{\code{xmap}}{cross mapping performance}
\item{\code{varname}}{name of target variable}
\item{\code{method}}{method of cross mapping}
\item{\code{tau}}{step of time lag}
}
}
\description{
intersection cardinality
}
\examples{
sim = logistic_map(x = 0.4,y = 0.4,step = 45,beta_xy = 0.5,beta_yx = 0)
ic(sim,"x","y",E = 4,k = 15:30,threads = 1)

}
\references{
Tao, P., Wang, Q., Shi, J., Hao, X., Liu, X., Min, B., Zhang, Y., Li, C., Cui, H., Chen, L., 2023. Detecting dynamical causality by intersection cardinal concavity. Fundamental Research.
}
