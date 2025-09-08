/**
 * Upgrade rule to notify users that events have been updated for clusters.
 * @param {*} api
 * @param {*} context
 * @returns A string mentioning the changes to cluster events made on a .zap file
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
      /* The following clusters no longer have certain events
      - COMMODITY_PRICE_CLUSTER
      - ELECTRICAL_GRID_CONDITIONS_CLUSTER
      */
      if (parseInt(clusters[j].code) == 0x0095) {
        let events = await api.events(context, epts[i], clusters[j])
        for (let k = 0; k < events.length; k++) {
            // ForecastChange event is no longer available
            if (parseInt(events[k].code) == 0x0001) {
                resMsg += `${events[k].name} event of ${clusters[j].name} cluster on endpoint ${epts[i].endpointIdentifier} is no longer available.\n`
            }
        }
      } else if (parseInt(clusters[j].code) == 0x0095) {
        let events = await api.events(context, epts[i], clusters[j])
        for (let k = 0; k < events.length; k++) {
            // ForecastConditionsChanged event is no longer available
            if (parseInt(events[k].code) == 0x0001) {
                resMsg += `${events[k].name} event of ${clusters[j].name} cluster on endpoint ${epts[i].endpointIdentifier} is no longer available.\n`
            }
        }
      }
    }
  }
  // Status can be 'nothing', 'automatic', 'user_verification', 'impossible'.
  if (resMsg) {
    return { message : resMsg, status : 'automatic' };
  }
}

exports.postLoad = postLoad
