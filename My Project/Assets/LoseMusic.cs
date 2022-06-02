using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class LoseMusic : MonoBehaviour
{
    // Start is called before the first frame update
    void Start()
    {
        AkSoundEngine.PostEvent("Lose", this.gameObject);
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
