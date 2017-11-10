import numpy as np
import pandas as pd
from subprocess import check_output
from io import StringIO

#TODO
#    Add parameters on histo
#        Duration
#        Binning
#        SyncDivider
#        SyncCFDZeroCross
#        SyncCFDLevel
#        SyncChannelOffset
#        InputCFDZeroCross
#        InputCFDLevel
#        InputChannelOffset        
#    Read detectors caracteristics (binning, time resolution) from the compiled c code ?
#        Need to change the c code
#            Output the time resolution
#            Output all the same parameters than from the "save in file" version ?
#    Compute a time axis ?
#        Need to change the c code (time resolution)

def histo():
    """
    Return the array containing the histogram of all available channels.

    This function depends on the (forked) executable histomode compiled from c.
    """
    out = check_output(["histomode"])
    df = pd.read_csv(StringIO(out.decode('utf-8')))
    return np.array(df)
