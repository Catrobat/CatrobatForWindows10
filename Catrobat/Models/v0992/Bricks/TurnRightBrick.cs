﻿using Catrobat_Player.NativeComponent;
using System.Xml.Serialization;
using System.Linq;

namespace Catrobat.Models.v0992
{
    partial class TurnRightBrick : ITurnRightBrick
    {
        #region NativeComponent
        [XmlIgnore]
        public IFormulaTree Rotation
        {
            get
            {
                return formulaList.FirstOrDefault(x => x.category == "TURN_RIGHT_DEGREES");
            }
            set { }
        }
        #endregion
    }
}
