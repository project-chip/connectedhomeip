/**
 * Upgrade rule to notify users that attributes have been updated for clusters.
 * @param {*} api
 * @param {*} context
 * @returns A string mentioning the changes to cluster attributes made on a .zap file
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
      /* The following clusters no longer have certain attributes
      - DESCRIPTOR_CLUSTER
      - MODE_SELECT_CLUSTER
      */
      if (parseInt(clusters[j].code) == 0x001d) {
        let attributes = await api.attributes(context, epts[i], clusters[j])
        for (let k = 0; k < attributes.length; k++) {
            // EventList attribute is no longer available
            if (parseInt(attributes[k].code) == 65530) {
                resMsg += `${attributes[k].name} attribute of ${clusters[j].name} cluster on endpoint ${epts[i].endpointIdentifier} is no longer available.\n`
            }
        }
      } else if (parseInt(clusters[j].code) == 0x0050) {
        let attributes = await api.attributes(context, epts[i], clusters[j])
        for (let k = 0; k < attributes.length; k++) {
            // ManufacturerExtension attribute is no longer available
            if (parseInt(attributes[k].code) == 4293984257) {
                resMsg += `${attributes[k].name} attribute of ${clusters[j].name} cluster on endpoint ${epts[i].endpointIdentifier} is no longer available.\n`
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
