/**
 * Upgrade rule to notify users that commands have been updated for clusters.
 * @param {*} api
 * @param {*} context
 * @returns A string mentioning the changes to cluster commands made on a .zap file
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
      /* The following clusters no longer have certain commands
      - OPERATIONAL_STATE_OVEN_CLUSTER
      */
      if (parseInt(clusters[j].code) == 0x0048) {
        let commands = await api.commands(context, epts[i], clusters[j])
        for (let k = 0; k < commands.length; k++) {
            // Pause command is no longer available
            // Resume command is no longer available
            if (parseInt(commands[k].code) == 1 || parseInt(commands[k].code) == 3) {
                resMsg += `${commands[k].name} command of ${clusters[j].name} cluster on endpoint ${epts[i].endpointIdentifier} is no longer available.\n`
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
