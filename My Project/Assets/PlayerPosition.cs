using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerPosition : MonoBehaviour
{
    public GameObject Player;
    public float playerPosition;
   
    // Start is called before the first frame update
    void Start()
    {
        GameObject.FindGameObjectsWithTag("Player");
    }

    // Update is called once per frame
    void Update()
    {
       
        playerPosition = Player.transform.position.y;
        AkSoundEngine.SetRTPCValue("playerPosition", playerPosition);
    }
}
