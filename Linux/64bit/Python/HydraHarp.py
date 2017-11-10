import numpy as np
import pandas as pd
from subprocess import check_output
from io import StringIO

def histo():
    """
    Return the array containing the histogram of all available channels.

    This function depends on the (forked) executable histomode compiled from c.
    """
    out = check_output(["histomode"])
    df = pd.read_csv(StringIO(out.decode('utf-8')))
    return np.array(df)
