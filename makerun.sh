echo -ne "" > ./runs.txt

OPP_DIR=${1:-/opt/omnetpp}
NETWORKS=$2
LIB=$3


for i in $(seq 0 9); do
  echo ". '${OPP_DIR}/bin/opp_run' -u Cmdenv -n '${NETWORKS}' -l ${LIB} omnetpp.ini -c SimpleHighNumberCACC -r $i" >> ./runs.txt
  echo ". '${OPP_DIR}/bin/opp_run' -u Cmdenv -n '${NETWORKS}' -l ${LIB} omnetpp.ini -c SimpleHighNumberPLOEG -r $i" >> ./runs.txt
done