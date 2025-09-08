/**
 * Upgrade rule to notify users that api maturity has been changed for clusters
 * and it is no longer provisional.
 * @param {*} api
 * @param {*} context
 * @returns A string mentioning the cluster API maturity changes made to a .zap file
 */
async function postLoad(api, context)
{
  let resMsg = ''
  let epts   = await api.endpoints(context)
  for (let i = 0; i < epts.length; i++)
  {
    let clusters = await api.clusters(context, epts[i])
    for (let j = 0; j < clusters.length; j++)
    {
      /* The following clusters are no longer provisional
      - WIFI_NETWORK_MANAGEMENT_CLUSTER
      - THREAD_BORDER_ROUTER_MANAGEMENT_CLUSTER
      - THREAD_NETWORK_DIRECTORY_CLUSTER
      */
      if ((parseInt(clusters[j].code) == 0x0451 || parseInt(clusters[j].code) == 0x0452 || parseInt(clusters[j].code) == 0x0453)
          && clusters[j].apiMaturity == 'provisional') {
        resMsg += `${clusters[j].name} cluster on endpoint ${epts[i].endpointIdentifier} is no longer provisional.\n`
      }
    }
  }
  // Status can be 'nothing', 'automatic', 'user_verification', 'impossible'.
  if (resMsg) {
    return { message : resMsg, status : 'automatic' };
  }
}

exports.postLoad = postLoad
